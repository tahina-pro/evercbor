module CBOR.Spec.Raw.Base
include CBOR.Spec.Constants
open CBOR.Spec.Util

module U8 = FStar.UInt8
module U64 = FStar.UInt64

(* Raw data items, disregarding ordering of map entries *)

let nlist ([@@@strictly_positive] t: eqtype) (n: nat) : Tot eqtype = (l: list t { List.Tot.length l == n })

type integer_size = (n: nat { n <= 4 })

open FStar.Mul

let raw_uint64_size_prop (size: integer_size) (value: U64.t) : Tot prop =
  if size = 0
  then U64.v value <= U8.v max_simple_value_additional_info
  else U64.v value < pow2 (8 * pow2 (size - 1))

type raw_uint64 = {
  size: integer_size;
  value: (value: U64.t { raw_uint64_size_prop size value })
}

let _ = assert_norm (8 * pow2 3 == 64)

type raw_data_item : eqtype
= | Simple: (v: simple_value) -> raw_data_item
  | Int64: (typ: major_type_uint64_or_neg_int64) -> (v: raw_uint64) -> raw_data_item
  | String: (typ: major_type_byte_string_or_text_string) -> (len: raw_uint64) -> (v: Seq.lseq U8.t (U64.v len.value)) -> raw_data_item // Section 3.1: "a string containing an invalid UTF-8 sequence is well-formed but invalid", so we don't care about UTF-8 specifics here.
  | Array: (len: raw_uint64) -> (v: nlist raw_data_item (U64.v len.value)) -> raw_data_item
  | Map: (len: raw_uint64) -> (v: nlist (raw_data_item & raw_data_item) (U64.v len.value)) -> raw_data_item
  | Tagged: (tag: raw_uint64) -> (v: raw_data_item) -> raw_data_item
//  | Float: (v: Float.float) -> raw_data_item // TODO

let dummy_raw_data_item : Ghost.erased raw_data_item =
  Int64 cbor_major_type_uint64 { size = 0; value = 0uL }

let raw_uint64_equiv (x1 x2: raw_uint64) : Tot bool =
  x1.value = x2.value

val raw_equiv (l1 l2: raw_data_item) : Tot bool

val raw_equiv_eq (l1 l2: raw_data_item) : Lemma
  (raw_equiv l1 l2 == begin match l1, l2 with
  | Simple v1, Simple v2 -> v1 = v2
  | Int64 ty1 v1, Int64 ty2 v2 -> ty1 = ty2 && raw_uint64_equiv v1 v2
  | String ty1 len1 v1, String ty2 len2 v2 -> ty1 = ty2 && raw_uint64_equiv len1 len2 && v1 = v2
  | Array len1 v1, Array len2 v2 -> raw_uint64_equiv len1 len2 && list_for_all2 raw_equiv v1 v2
  | Map len1 v1, Map len2 v2 ->
    raw_uint64_equiv len1 len2 &&
    list_for_all_exists (holds_on_pair2 raw_equiv) v1 v2 &&
    list_for_all_exists (holds_on_pair2 raw_equiv) v2 v1
  | Tagged tag1 v1, Tagged tag2 v2 ->
    raw_uint64_equiv tag1 tag2 &&
    raw_equiv v1 v2
  | _ -> false
  end)

val raw_equiv_sym (l1 l2: raw_data_item) : Lemma
  (ensures (raw_equiv l1 l2 == raw_equiv l2 l1))

noextract
let get_major_type
  (d: raw_data_item)
: Tot major_type_t
= match d with
  | Simple _ -> cbor_major_type_simple_value
  | Int64 m _ -> m
  | String m _ _ -> m
  | Array _ _ -> cbor_major_type_array
  | Map _ _ -> cbor_major_type_map
  | Tagged _ _ -> cbor_major_type_tagged

noextract
val holds_on_raw_data_item
  (p: (raw_data_item -> bool))
  (x: raw_data_item)
: Tot bool

noextract
let holds_on_raw_data_item'
  (p: (raw_data_item -> bool))
  (x: raw_data_item)
: Tot bool
= p x &&
  begin match x with
  | Array _ l -> List.Tot.for_all (holds_on_raw_data_item p) l
  | Map _ l ->
    List.Tot.for_all (holds_on_pair (holds_on_raw_data_item p)) l
  | Tagged _ v -> holds_on_raw_data_item p v
  | _ -> true
  end

val holds_on_raw_data_item_eq
  (p: (raw_data_item -> bool))
  (x: raw_data_item)
: Lemma
  (holds_on_raw_data_item p x == holds_on_raw_data_item' p x)

let rec holds_on_raw_data_item_andp
  (p1 p2: (raw_data_item -> bool))
  (x: raw_data_item)
: Lemma
  (ensures (
    holds_on_raw_data_item (andp p1 p2) x == (holds_on_raw_data_item p1 x && holds_on_raw_data_item p2 x)
  ))
  (decreases x)
= holds_on_raw_data_item_eq (andp p1 p2) x;
  holds_on_raw_data_item_eq p1 x;
  holds_on_raw_data_item_eq p2 x;
  match x with
  | Array _ l ->
    list_for_all_ext (holds_on_raw_data_item (andp p1 p2)) (andp (holds_on_raw_data_item p1) (holds_on_raw_data_item p2)) l (fun x -> holds_on_raw_data_item_andp p1 p2 x);
    list_for_all_andp (holds_on_raw_data_item p1) (holds_on_raw_data_item p2) l
  | Map _ l ->
    list_for_all_ext (holds_on_pair (holds_on_raw_data_item (andp p1 p2))) (andp (holds_on_pair (holds_on_raw_data_item p1)) (holds_on_pair (holds_on_raw_data_item p2))) l (fun x ->
      holds_on_raw_data_item_andp p1 p2 (fst x);
      holds_on_raw_data_item_andp p1 p2 (snd x)
    );
    list_for_all_andp (holds_on_pair (holds_on_raw_data_item p1)) (holds_on_pair (holds_on_raw_data_item p2)) l
  | Tagged _ x' -> holds_on_raw_data_item_andp p1 p2 x'
  | _ -> ()

let rec holds_on_raw_data_item_implies
  (p1 p2: (raw_data_item -> bool))
  (prf: ((x': raw_data_item) -> Lemma
    (requires (holds_on_raw_data_item p1 x' == true))
    (ensures (p2 x' == true))
  ))
  (x: raw_data_item)
: Lemma
  (requires (holds_on_raw_data_item p1 x))
  (ensures (holds_on_raw_data_item p2 x == true))
  (decreases x)
= holds_on_raw_data_item_eq p1 x;
  holds_on_raw_data_item_eq p2 x;
  prf x;
  match x with
  | Array _ v ->
    list_for_all_implies (holds_on_raw_data_item p1) (holds_on_raw_data_item p2) v (fun x -> holds_on_raw_data_item_implies p1 p2 prf x)
  | Tagged _ v -> holds_on_raw_data_item_implies p1 p2 prf v
  | Map _ v ->
    list_for_all_implies (holds_on_pair (holds_on_raw_data_item p1)) (holds_on_pair (holds_on_raw_data_item p2)) v (fun x ->
      holds_on_raw_data_item_implies p1 p2 prf (fst x);
      holds_on_raw_data_item_implies p1 p2 prf (snd x)
    )
  | _ -> ()

let holds_on_raw_data_item_eq_simple
  (p: (raw_data_item -> bool))
  (v: simple_value)
: Lemma
  (holds_on_raw_data_item p (Simple v) == p (Simple v))
  [SMTPat (holds_on_raw_data_item p (Simple v))]
= holds_on_raw_data_item_eq p (Simple v)

let holds_on_raw_data_item_eq_int64
  (p: (raw_data_item -> bool))
  (typ: major_type_uint64_or_neg_int64)
  (v: raw_uint64)
: Lemma
  (holds_on_raw_data_item p (Int64 typ v) == p (Int64 typ v))
  [SMTPat (holds_on_raw_data_item p (Int64 typ v))]
= holds_on_raw_data_item_eq p (Int64 typ v)

let holds_on_raw_data_item_eq_string
  (p: (raw_data_item -> bool))
  (typ: major_type_byte_string_or_text_string)
  (len: raw_uint64)
  (v: Seq.lseq U8.t (U64.v len.value))
: Lemma
  (holds_on_raw_data_item p (String typ len v) == p (String typ len v))
  [SMTPat (holds_on_raw_data_item p (String typ len v))]
= holds_on_raw_data_item_eq p (String typ len v)

let holds_on_raw_data_item_eq_array
  (p: (raw_data_item -> bool))
  (len: raw_uint64)
  (v: nlist raw_data_item (U64.v len.value))
: Lemma
  (holds_on_raw_data_item p (Array len v) == (p (Array len v) && List.Tot.for_all (holds_on_raw_data_item p) v))
  [SMTPat (holds_on_raw_data_item p (Array len v))]
= holds_on_raw_data_item_eq p (Array len v)

let holds_on_raw_data_item_eq_map
  (p: (raw_data_item -> bool))
  (len: raw_uint64)
  (v: nlist (raw_data_item & raw_data_item) (U64.v len.value))
: Lemma
  (holds_on_raw_data_item p (Map len v) == (p (Map len v) && List.Tot.for_all (holds_on_pair (holds_on_raw_data_item p)) v))
  [SMTPat (holds_on_raw_data_item p (Map len v))]
= holds_on_raw_data_item_eq p (Map len v)

let holds_on_raw_data_item_eq_tagged
  (p: (raw_data_item -> bool))
  (tag: raw_uint64)
  (v: raw_data_item)
: Lemma
  (holds_on_raw_data_item p (Tagged tag v) <==> (p (Tagged tag v) && holds_on_raw_data_item p v))
  [SMTPat (holds_on_raw_data_item p (Tagged tag v))]
= holds_on_raw_data_item_eq p (Tagged tag v)

noextract
let map_entry_order
  (#key: Type)
  (key_order: (key -> key -> bool))
  (value: Type)
  (m1: (key & value))
  (m2: (key & value))
: Tot bool
= key_order (fst m1) (fst m2)

noextract
let raw_data_item_sorted_elem (order: (raw_data_item -> raw_data_item -> bool)) (x: raw_data_item) : Tot bool
= match x with
  | Map _ l ->
      FStar.List.Tot.sorted (map_entry_order order _) l
  | _ -> true

noextract
let raw_data_item_sorted (order: (raw_data_item -> raw_data_item -> bool)) : Tot (raw_data_item -> bool)
= holds_on_raw_data_item (raw_data_item_sorted_elem order)

let raw_uint64_optimal (x: raw_uint64) : Tot bool =
  ((U64.v x.value <= U8.v max_simple_value_additional_info) = (x.size = 0)) &&
  begin
    if x.size <= 1
    then true
    else pow2 (8 * pow2 (x.size - 2)) <= U64.v x.value
  end

let raw_uint64_optimal_unique (x1 x2: raw_uint64) : Lemma
  (requires (raw_uint64_optimal x1 /\ raw_uint64_optimal x2 /\ raw_uint64_equiv x1 x2))
  (ensures (x1 == x2))
= ()

let rec raw_uint64_optimize (x: raw_uint64) : Pure raw_uint64
  (requires True)
  (ensures (fun x' -> raw_uint64_equiv x x' /\ raw_uint64_optimal x'))
  (decreases x.size)
= if U64.v x.value <= U8.v max_simple_value_additional_info
  then { x with size = 0 }
  else if x.size <= 1
  then x
  else if pow2 (8 * pow2 (x.size - 2)) <= U64.v x.value
  then x
  else raw_uint64_optimize { x with size = x.size - 1 }

let raw_data_item_ints_optimal_elem (x: raw_data_item) : Tot bool =
  match x with
  | Int64 _ v
  | String _ v _
  | Array v _
  | Map v _
  | Tagged v _
    -> raw_uint64_optimal v
  | Simple _ -> true

let raw_data_item_ints_optimal : raw_data_item -> Tot bool =
  holds_on_raw_data_item raw_data_item_ints_optimal_elem

let valid_raw_data_item_map
  (v: list (raw_data_item & raw_data_item))
: Tot bool
= list_no_setoid_repeats (map_entry_order raw_equiv _) v

let valid_raw_data_item_elem
  (l: raw_data_item)
: Tot bool
= match l with
  | Map _ v -> valid_raw_data_item_map v
  | _ -> true

let valid_raw_data_item
  (l: raw_data_item)
: Tot bool
= holds_on_raw_data_item valid_raw_data_item_elem l

let rec raw_equiv_sorted_optimal
  (order: raw_data_item -> raw_data_item -> bool {
    order_irrefl order /\
    order_trans order
  })
  (x1 x2: raw_data_item)
: Lemma
  (requires (
    raw_equiv x1 x2 /\
    raw_data_item_sorted order x1 /\
    raw_data_item_sorted order x2 /\
    raw_data_item_ints_optimal x1 /\
    raw_data_item_ints_optimal x2
  ))
  (ensures (x1 == x2))
  (decreases x1)
= raw_equiv_eq x1 x2;
  holds_on_raw_data_item_eq (raw_data_item_sorted_elem order) x1;
  holds_on_raw_data_item_eq (raw_data_item_sorted_elem order) x2;
  holds_on_raw_data_item_eq raw_data_item_ints_optimal_elem x1;
  holds_on_raw_data_item_eq raw_data_item_ints_optimal_elem x2;
  match x1, x2 with
  | Simple _, Simple _ -> ()
  | Int64 _ v1, Int64 _ v2 ->
    raw_uint64_optimal_unique v1 v2
  | String _ v1 _, String _ v2 _ ->
    raw_uint64_optimal_unique v1 v2
  | Tagged tag1 v1, Tagged tag2 v2 ->
    raw_uint64_optimal_unique tag1 tag2;
    raw_equiv_sorted_optimal order v1 v2
  | Array len1 l1, Array len2 l2 ->
    raw_uint64_optimal_unique len1 len2;
    assert_norm (raw_data_item_ints_optimal == holds_on_raw_data_item raw_data_item_ints_optimal_elem);
    list_for_all2_prod (raw_data_item_sorted order) (raw_data_item_sorted order) l1 l2;
    list_for_all2_prod raw_data_item_ints_optimal raw_data_item_ints_optimal l1 l2;
    list_for_all2_andp_intro
      (prodp (raw_data_item_sorted order) (raw_data_item_sorted order))
      (prodp raw_data_item_ints_optimal raw_data_item_ints_optimal)
      l1 l2;
    list_for_all2_andp_intro
      (andp2
        (prodp (raw_data_item_sorted order) (raw_data_item_sorted order))
        (prodp raw_data_item_ints_optimal raw_data_item_ints_optimal))
      raw_equiv
      l1 l2;
    list_for_all2_implies
      (andp2
        (andp2
          (prodp (raw_data_item_sorted order) (raw_data_item_sorted order))
          (prodp raw_data_item_ints_optimal raw_data_item_ints_optimal))
        raw_equiv
      )
      ( = )
      l1 l2
      (fun x1' x2' ->
        raw_equiv_sorted_optimal order x1' x2'
      );
    list_for_all2_equals l1 l2
  | Map len1 l1, Map len2 l2 ->
    raw_uint64_optimal_unique len1 len2;
    assert_norm (raw_data_item_ints_optimal == holds_on_raw_data_item raw_data_item_ints_optimal_elem);
    list_for_all_andp (holds_on_pair (raw_data_item_sorted order)) (holds_on_pair raw_data_item_ints_optimal) l1;
    list_for_all_andp (holds_on_pair (raw_data_item_sorted order)) (holds_on_pair raw_data_item_ints_optimal) l2;
    list_for_all_exists_prodp
      (holds_on_pair2 raw_equiv)
      (andp
        (holds_on_pair (raw_data_item_sorted order))
        (holds_on_pair raw_data_item_ints_optimal)
      )
      (andp
        (holds_on_pair (raw_data_item_sorted order))
        (holds_on_pair raw_data_item_ints_optimal)
      )
      l1 l2;
    list_for_all_exists_implies
      (andp2
        (holds_on_pair2 raw_equiv)
        (prodp
          (andp
            (holds_on_pair (raw_data_item_sorted order))
            (holds_on_pair raw_data_item_ints_optimal)
          )
          (andp
            (holds_on_pair (raw_data_item_sorted order))
            (holds_on_pair raw_data_item_ints_optimal)
          )
        )
      )
      ( = )
      l1 l2
      (fun x1 x2 ->
        raw_equiv_sorted_optimal order (fst x1) (fst x2);
        raw_equiv_sorted_optimal order (snd x1) (snd x2)
      );
    list_for_all_exists_equal_eq l1 l2;
    list_for_all_exists_prodp
      (holds_on_pair2 raw_equiv)
      (andp
        (holds_on_pair (raw_data_item_sorted order))
        (holds_on_pair raw_data_item_ints_optimal)
      )
      (andp
        (holds_on_pair (raw_data_item_sorted order))
        (holds_on_pair raw_data_item_ints_optimal)
      )
      l2 l1;
    list_for_all_exists_implies
      (andp2
        (holds_on_pair2 raw_equiv)
        (prodp
          (andp
            (holds_on_pair (raw_data_item_sorted order))
            (holds_on_pair raw_data_item_ints_optimal)
          )
          (andp
            (holds_on_pair (raw_data_item_sorted order))
            (holds_on_pair raw_data_item_ints_optimal)
          )
        )
      )
      ( = )
      l2 l1
      (fun x2 x1 ->
        raw_equiv_sym (fst x2) (fst x1);
        raw_equiv_sorted_optimal order (fst x1) (fst x2);
        raw_equiv_sym (snd x2) (snd x1);
        raw_equiv_sorted_optimal order (snd x1) (snd x2)
      );
    list_for_all_exists_equal_eq l2 l1;
    list_sorted_ext_eq (map_entry_order order _) l1 l2

let rec raw_data_item_sorted_optimal_valid_aux
  (order: (raw_data_item -> raw_data_item -> bool) {
    order_irrefl order /\
    order_trans order
  })
  (l: list (raw_data_item & raw_data_item))
: Lemma
  (requires (
    List.Tot.for_all (holds_on_pair (raw_data_item_sorted order)) l /\
    List.Tot.for_all (holds_on_pair raw_data_item_ints_optimal) l /\
    FStar.List.Tot.sorted (map_entry_order order _) l
  ))
  (ensures (
    list_no_setoid_repeats (map_entry_order raw_equiv _) l
  ))
  (decreases l)
= match l with
  | [] -> ()
  | a :: q ->
    raw_data_item_sorted_optimal_valid_aux order q;
    if List.Tot.existsb (map_entry_order raw_equiv _ a) q
    then begin
      let a' = list_existsb_elim (map_entry_order raw_equiv _ a) q in
      list_sorted_memP (map_entry_order order _) a q a';
      List.Tot.for_all_mem (holds_on_pair (raw_data_item_sorted order)) q;
      List.Tot.for_all_mem (holds_on_pair (raw_data_item_ints_optimal)) q;
      raw_equiv_sorted_optimal order (fst a) (fst a')
    end
    else ()

let raw_data_item_sorted_optimal_valid
  (order: raw_data_item -> raw_data_item -> bool {
    order_irrefl order /\
    order_trans order
  })
  (x1: raw_data_item)
: Lemma
  (requires (
    raw_data_item_sorted order x1 /\
    raw_data_item_ints_optimal x1
  ))
  (ensures (valid_raw_data_item x1))
= holds_on_raw_data_item_andp (raw_data_item_sorted_elem order) raw_data_item_ints_optimal_elem x1;
  holds_on_raw_data_item_implies
    (andp (raw_data_item_sorted_elem order) raw_data_item_ints_optimal_elem)
    valid_raw_data_item_elem
    (fun x ->
      match x with
      | Map len v ->
        holds_on_raw_data_item_andp (raw_data_item_sorted_elem order) raw_data_item_ints_optimal_elem x;
          holds_on_raw_data_item_eq (raw_data_item_sorted_elem order) (Map len v);
          holds_on_raw_data_item_eq (raw_data_item_ints_optimal_elem) (Map len v);
          assert (List.Tot.for_all (holds_on_pair (holds_on_raw_data_item raw_data_item_ints_optimal_elem)) v);
          assert_norm (raw_data_item_ints_optimal == holds_on_raw_data_item raw_data_item_ints_optimal_elem);
          assert (List.Tot.for_all (holds_on_pair raw_data_item_ints_optimal) v);
          raw_data_item_sorted_optimal_valid_aux order v
      | _ -> ()
    )
    x1

val raw_data_item_size
  (x: raw_data_item)
: Tot nat

val raw_data_item_size_eq
  (x: raw_data_item)
: Lemma
  (raw_data_item_size x == begin match x with
  | Array _ v -> 2 + list_sum raw_data_item_size v
  | Map _ v -> 2 + list_sum (pair_sum raw_data_item_size raw_data_item_size) v
  | Tagged _ v -> 2 + raw_data_item_size v
  | _ -> 1
  end)