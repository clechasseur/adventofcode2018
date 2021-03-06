// Copyright (c) 2016, Coveo Solutions Inc.
// Distributed under the MIT license (see LICENSE).

// Implementation details of coveo::linq operators.

#ifndef COVEO_LINQ_DETAIL_H
#define COVEO_LINQ_DETAIL_H

#include <coveo/enumerable/detail/enumerable_detail.h>
#include <coveo/linq/exception.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <type_traits>
#include <vector>
#include <utility>

namespace coveo {
namespace linq {
namespace detail {

// Traits class used by LINQ operators. A shorthand for enumerable's seq_element_traits
// that infers the sequence's value_type from the return value of its iterators.
// Also provides the type of iterator used by the sequence.
template<typename Seq>
struct seq_traits : public coveo::detail::seq_element_traits<decltype(*std::begin(std::declval<Seq&>()))>
{
    typedef typename std::decay<decltype(std::begin(std::declval<Seq&>()))>::type iterator_type;    // Type of iterator used by the sequence
};
template<typename Seq> struct seq_traits<Seq&> : seq_traits<Seq> { };
template<typename Seq> struct seq_traits<Seq&&> : seq_traits<Seq> { };
template<typename Seq> struct seq_traits<std::reference_wrapper<Seq>> : seq_traits<Seq> { };

// Proxy comparator that references an external predicate.
// Allows instances of lambdas to be used as predicates for sets, for instance.
template<typename Pred>
class proxy_cmp
{
private:
    const typename std::decay<Pred>::type* ppred_;

public:
    explicit proxy_cmp(const Pred& pred)
        : ppred_(std::addressof(pred)) { }
    
    template<typename T, typename U>
    auto operator()(T&& left, U&& right) const -> decltype((*ppred_)(std::forward<T>(left), std::forward<U>(right))) {
        return (*ppred_)(std::forward<T>(left), std::forward<U>(right));
    }
};

// Comparator that receives pointers to elements and
// dereferences them, then calls another predicate.
template<typename Pred>
class deref_cmp
{
private:
    Pred pred_;

public:
    explicit deref_cmp(Pred&& pred)
        : pred_(std::forward<Pred>(pred)) { }

    template<typename T, typename U>
    auto operator()(T* const pleft, U* const pright) const -> decltype(pred_(*pleft, *pright)) {
        return pred_(*pleft, *pright);
    }
};

// Selector implementation that can be used with some operators
// that return an element and its index, when the index is not needed.
template<typename Selector>
class indexless_selector_proxy
{
private:
    Selector sel_;  // Selector that doesn't care about index

public:
    explicit indexless_selector_proxy(Selector&& sel)
        : sel_(std::forward<Selector>(sel)) { }

    template<typename T>
    auto operator()(T&& element, std::size_t) -> decltype(sel_(std::forward<T>(element))) {
        return sel_(std::forward<T>(element));
    }
};

// Next delegate implementation that takes a sequence of pointers
// and fronts a sequence of references by dereferencing said pointers.
// Meant to be used to construct enumerables.
template<typename Seq>
class deref_next_impl
{
public:
    // Type of iterator used by the sequence.
    typedef typename seq_traits<Seq>::iterator_type iterator_type;

private:
    // Struct storing sequence and ending. Shared among delegates.
    struct deref_info {
        Seq seq_;               // Sequence of pointers.
        iterator_type iend_;    // Iterator pointing at end of seq_.

        explicit deref_info(Seq&& seq)
            : seq_(std::forward<Seq>(seq)),
              iend_(std::end(seq_)) { }

        // Cannot be copy/move, stored in a shared_ptr
        deref_info(const deref_info&) = delete;
        deref_info& operator=(const deref_info&) = delete;
    };
    typedef std::shared_ptr<deref_info> deref_info_sp;

    deref_info_sp spinfo_;      // Shared sequence info.
    iterator_type icur_;        // Iterator pointing at current element.

public:
    explicit deref_next_impl(Seq&& seq)
        : spinfo_(std::make_shared<deref_info>(std::forward<Seq>(seq))),
          icur_(std::begin(spinfo_->seq_)) { }

    auto operator()() -> typename seq_traits<Seq>::value_type {
        typename seq_traits<Seq>::value_type pobj = nullptr;
        if (icur_ != spinfo_->iend_) {
            pobj = *icur_;
            ++icur_;
        }
        return pobj;
    }
};
template<typename Seq>
deref_next_impl<Seq> make_deref_next_impl(Seq&& seq) {
    return deref_next_impl<Seq>(std::forward<Seq>(seq));
}

// Transparent predicate that returns values unmodified.
template<typename = void>
struct identity {
    template<typename T>
    auto operator()(T&& obj) const -> decltype(std::forward<T>(obj)) {
        return std::forward<T>(obj);
    }
};

// Transparent binary predicate that returns a pair
// of its two arguments.
template<typename = void>
struct pair_of {
    template<typename T, typename U>
    auto operator()(T&& obj1, U&& obj2) const -> std::pair<T, U> {
        return std::pair<T, U>(std::forward<T>(obj1), std::forward<U>(obj2));
    }
};

// Transparent implementations of less and greater, like those in C++14.
template<typename = void>
struct less {
    template<typename T, typename U>
    auto operator()(T&& left, U&& right) const -> decltype(std::forward<T>(left) < std::forward<U>(right)) {
        return std::forward<T>(left) < std::forward<U>(right);
    }
};
template<typename = void>
struct greater {
    template<typename T, typename U>
    auto operator()(T&& left, U&& right) const -> decltype(std::forward<T>(left) > std::forward<U>(right)) {
        return std::forward<T>(left) > std::forward<U>(right);
    }
};

// Creates an object and stores it in a unique_ptr, like std::make_unique from C++14.
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Helper that reserves space in a container based on the number of elements in a sequence
// if it's possible to do so quickly (e.g. with random-access iterators or size())
template<typename C, typename Seq>
auto try_reserve(C& cnt, const Seq& seq) -> typename std::enable_if<coveo::detail::is_enumerable<Seq>::value, bool>::type
{
    const bool can_reserve = seq.has_fast_size();
    if (can_reserve) {
        cnt.reserve(seq.size());
    }
    return can_reserve;
}
template<typename C, typename Seq>
auto try_reserve(C& cnt, const Seq& seq) -> typename std::enable_if<!coveo::detail::is_enumerable<Seq>::value &&
                                                                    coveo::detail::has_size_const_method<Seq>::value, bool>::type
{
    cnt.reserve(seq.size());
    return true;
}
template<typename C, typename Seq>
auto try_reserve(C& cnt, const Seq& seq) -> typename std::enable_if<!coveo::detail::has_size_const_method<Seq>::value &&
                                                                    std::is_base_of<std::random_access_iterator_tag,
                                                                                    typename std::iterator_traits<typename std::decay<decltype(std::begin(std::declval<Seq>()))>::type>::iterator_category>::value,
                                                                    bool>::type
{
    cnt.reserve(std::distance(std::begin(seq), std::end(seq)));
    return true;
}
template<typename C, typename Seq>
auto try_reserve(C&, const Seq&) -> typename std::enable_if<!coveo::detail::has_size_const_method<typename std::decay<Seq>::type>::value &&
                                                            !std::is_base_of<std::random_access_iterator_tag,
                                                                             typename std::iterator_traits<typename std::decay<decltype(std::begin(std::declval<Seq>()))>::type>::iterator_category>::value,
                                                            bool>::type
{
    // Can't reserve, no fast way of doing so
    return false;
}

// Helper that returns a size_delegate for a sequence if it's possible to quickly calculate
// its size (e.g. with random-access iterators or size())
template<typename T, typename Seq>
auto try_get_size_delegate(const Seq& seq) -> typename std::enable_if<coveo::detail::is_enumerable<Seq>::value,
                                                                      typename coveo::enumerable<T>::size_delegate>::type
{
    typename coveo::enumerable<T>::size_delegate siz;
    if (seq.has_fast_size()) {
        std::size_t size = seq.size();
        siz = [size]() -> std::size_t { return size; };
    }
    return siz;
}
template<typename T, typename Seq>
auto try_get_size_delegate(const Seq& seq) -> typename std::enable_if<!coveo::detail::is_enumerable<Seq>::value &&
                                                                      coveo::detail::has_size_const_method<Seq>::value,
                                                                      typename coveo::enumerable<T>::size_delegate>::type
{
    std::size_t size = seq.size();
    return [size]() -> std::size_t { return size; };
}
template<typename T, typename Seq>
auto try_get_size_delegate(const Seq& seq) -> typename std::enable_if<!coveo::detail::has_size_const_method<Seq>::value &&
                                                                      std::is_base_of<std::random_access_iterator_tag,
                                                                                      typename std::iterator_traits<typename std::decay<decltype(std::begin(std::declval<Seq>()))>::type>::iterator_category>::value,
                                                                      typename coveo::enumerable<T>::size_delegate>::type
{
    std::size_t size = static_cast<std::size_t>(std::distance(std::begin(seq), std::end(seq)));
    return [size]() -> std::size_t { return size; };
}
template<typename T, typename Seq>
auto try_get_size_delegate(const Seq&) -> typename std::enable_if<!coveo::detail::has_size_const_method<Seq>::value &&
                                                                  !std::is_base_of<std::random_access_iterator_tag,
                                                                                   typename std::iterator_traits<typename std::decay<decltype(std::begin(std::declval<Seq>()))>::type>::iterator_category>::value,
                                                                  typename coveo::enumerable<T>::size_delegate>::type
{
    // No way to quickly determine size, don't try
    return nullptr;
}

// Utility methods to throw LINQ-specific exceptions.
template<typename = void>
[[noreturn]] void throw_linq_empty_sequence() {
    throw empty_sequence("empty_sequence");
}
template<typename = void>
[[noreturn]] void throw_linq_out_of_range() {
    throw out_of_range("out_of_range");
}

// Implementation of aggregate operator (version with aggregate function only).
template<typename F>
class aggregate_impl_1
{
private:
    const F& agg_f_;    // Aggregate function.

public:
    explicit aggregate_impl_1(const F& agg_f)
        : agg_f_(agg_f) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename std::decay<decltype(*std::begin(seq))>::type {
        auto it = std::begin(seq);
        auto end = std::end(seq);
        if (it == end) {
            throw_linq_empty_sequence();
        }
        auto aggregate(*it);
        for (++it; it != end; ++it) {
            aggregate = agg_f_(aggregate, *it);
        }
        return aggregate;
    }
};

// Implementation of aggregate operator (version with aggregate function and initial value).
template<typename Acc, typename F>
class aggregate_impl_2
{
private:
    const Acc& seed_;   // Initial aggregate value.
    const F& agg_f_;    // Aggregate function.

public:
    aggregate_impl_2(const Acc& seed, const F& agg_f)
        : seed_(seed), agg_f_(agg_f) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> Acc {
        Acc aggregate(seed_);
        for (auto&& element : seq) {
            aggregate = agg_f_(aggregate, element);
        }
        return aggregate;
    }
};

// Implementation of aggregate operator (version with result function).
template<typename Acc, typename F, typename RF>
class aggregate_impl_3 : public aggregate_impl_2<Acc, F>
{
private:
    const RF& result_f_;    // Function to convert aggregate into final result.

public:
    aggregate_impl_3(const Acc& seed, const F& agg_f, const RF& result_f)
        : aggregate_impl_2<Acc, F>(seed, agg_f), result_f_(result_f) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(result_f_(std::declval<Acc>())) {
        return result_f_(aggregate_impl_2<Acc, F>::operator()(seq));
    }
};

// Implementation of all operator.
template<typename Pred>
class all_impl
{
private:
    const Pred& pred_;  // Predicate to satisfy.

public:
    explicit all_impl(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> bool {
        return std::all_of(std::begin(seq), std::end(seq), pred_);
    }
};

// Implementation of any operator.
template<typename = void>
class any_impl
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> bool {
        return std::begin(seq) != std::end(seq);
    }
};

// Implementation of average operator.
template<typename F>
class average_impl
{
private:
    const F& num_f_;    // Function to get numeric value for each element.

public:
    explicit average_impl(const F& num_f)
        : num_f_(num_f) { }

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> typename std::decay<decltype(num_f_(*std::begin(seq)))>::type
    {
        auto it = std::begin(seq);
        auto end = std::end(seq);
        if (it == end) {
            throw_linq_empty_sequence();
        }
        auto total = num_f_(*it);
        decltype(total) count = 1;
        for (++it; it != end; ++it) {
            total += num_f_(*it);
            ++count;
        }
        return total / count;
    }
};

// Selector implementation used by cast operator.
template<typename U>
class cast_selector
{
public:
    template<typename T>
    auto operator()(T&& obj) const -> U {
        return static_cast<U>(obj);
    }
};

// Implementation of concat operator.
template<typename Seq2>
class concat_impl
{
public:
    // Implementation of next delegate that concatenates two sequences
    template<typename Seq1>
    class next_impl
    {
    public:
        // Type of element returned by this next delegate. The elements will be const
        // if at least one sequence is const.
        typedef typename std::conditional<std::is_const<typename seq_traits<Seq1>::value_type>::value ||
                                          std::is_const<typename seq_traits<Seq2>::value_type>::value,
                                          typename seq_traits<Seq1>::const_value_type,
                                          typename seq_traits<Seq1>::value_type>::type  enum_type;

    private:
        // Type of iterators for both sequences
        typedef typename seq_traits<Seq1>::iterator_type first_iterator_type;
        typedef typename seq_traits<Seq2>::iterator_type second_iterator_type;

        // Information used to concatenate sequences. Shared among delegates.
        class concat_info
        {
        private:
            Seq1 seq1_;                     // First sequence to concatenate
            first_iterator_type iend1_;     // End of first sequence
            Seq2 seq2_;                     // Second sequence to concatenate
            second_iterator_type iend2_;    // End of second sequence

        public:
            concat_info(Seq1&& seq1, Seq2&& seq2)
                : seq1_(std::forward<Seq1>(seq1)),
                  iend1_(std::end(seq1_)),
                  seq2_(std::forward<Seq2>(seq2)),
                  iend2_(std::end(seq2_)) { }

            // Cannot move/copy, stored in a shared_ptr
            concat_info(const concat_info&) = delete;
            concat_info& operator=(const concat_info&) = delete;

            first_iterator_type first_begin() {
                return std::begin(seq1_);
            }
            second_iterator_type second_begin() {
                return std::begin(seq2_);
            }

            // Returns next element from one of the sequences or nullptr when done
            auto get_next(first_iterator_type& icur1, second_iterator_type& icur2)
                -> typename coveo::detail::seq_element_traits<enum_type>::pointer
            {
                // First return all elements from first sequence, then from second sequence.
                typename coveo::detail::seq_element_traits<enum_type>::pointer pobj = nullptr;
                if (icur1 != iend1_) {
                    typename coveo::detail::seq_element_traits<enum_type>::reference robj = *icur1;
                    pobj = std::addressof(robj);
                    ++icur1;
                } else if (icur2 != iend2_) {
                    typename coveo::detail::seq_element_traits<enum_type>::reference robj = *icur2;
                    pobj = std::addressof(robj);
                    ++icur2;
                }
                return pobj;
            }
        };
        typedef std::shared_ptr<concat_info> concat_info_sp;

        concat_info_sp spinfo_;         // Shared concat info
        first_iterator_type icur1_;     // Current position in first sequence
        second_iterator_type icur2_;    // Current position in second sequence

    public:
        next_impl(Seq1&& seq1, Seq2&& seq2)
            : spinfo_(std::make_shared<concat_info>(std::forward<Seq1>(seq1), std::forward<Seq2>(seq2))),
              icur1_(spinfo_->first_begin()), icur2_(spinfo_->second_begin()) { }

        auto operator()() -> decltype(spinfo_->get_next(icur1_, icur2_)) {
            return spinfo_->get_next(icur1_, icur2_);
        }
    };

private:
    Seq2 seq2_;     // Second sequence (possibly a ref)
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    explicit concat_impl(Seq2&& seq2)
        : seq2_(std::forward<Seq2>(seq2))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    concat_impl(const concat_impl&) = delete;
    concat_impl(concat_impl&&) = default;
    concat_impl& operator=(const concat_impl&) = delete;
    concat_impl& operator=(concat_impl&&) = default;

    template<typename Seq1>
    auto operator()(Seq1&& seq1)
        -> coveo::enumerable<typename next_impl<Seq1>::enum_type>
    {
        // Note: if seq2_ is not a ref, it is moved by the forward below
        // and becomes invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        auto siz1 = try_get_size_delegate<typename next_impl<Seq1>::enum_type>(seq1);
        auto siz2 = try_get_size_delegate<typename next_impl<Seq1>::enum_type>(seq2_);
        typename coveo::enumerable<typename next_impl<Seq1>::enum_type>::size_delegate siz;
        if (siz1 != nullptr && siz2 != nullptr) {
            std::size_t size = siz1() + siz2();
            siz = [size]() -> std::size_t { return size; };
        }
        return coveo::enumerable<typename next_impl<Seq1>::enum_type>(next_impl<Seq1>(std::forward<Seq1>(seq1),
                                                                                      std::forward<Seq2>(seq2_)),
                                                                      siz);
    }
};

// Implementation of contains operator (version with object only).
template<typename T>
class contains_impl_1
{
private:
    const T& obj_;  // Object to look for.

public:
    explicit contains_impl_1(const T& obj)
        : obj_(obj) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> bool {
        bool found = false;
        for (auto&& element : seq) {
            if (element == obj_) {
                found = true;
                break;
            }
        }
        return found;
    }
};

// Implementation of contains operator (version with object and predicate).
template<typename T, typename Pred>
class contains_impl_2
{
private:
    const T& obj_;      // Object to look for.
    const Pred& pred_;  // Predicate used to compare objects.

public:
    contains_impl_2(const T& obj, const Pred& pred)
        : obj_(obj), pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> bool {
        bool found = false;
        for (auto&& element : seq) {
            if (pred_(element, obj_)) {
                found = true;
                break;
            }
        }
        return found;
    }
};

// Implementation of count operator (version without parameters).
template<typename = void>
class count_impl_0
{
private:
    // Used if sequence has size() method
    template<typename Seq,
             typename = typename std::enable_if<coveo::detail::has_size_const_method<typename std::decay<Seq>::type>::value, void>::type>
    auto impl(Seq&& seq) -> std::size_t {
        return seq.size();
    }

    // Used otherwise (no choice but to use distance)
    template<typename Seq,
             typename _V = typename std::enable_if<!coveo::detail::has_size_const_method<typename std::decay<Seq>::type>::value, void*>::type>
    auto impl(Seq&& seq, _V = nullptr) -> std::size_t {
        return static_cast<std::size_t>(std::distance(std::begin(seq), std::end(seq)));
    }

public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> std::size_t {
        return impl(std::forward<Seq>(seq));
    }
};

// Implementation of count operator (version with predicate).
template<typename Pred>
class count_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy.

public:
    explicit count_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> std::size_t {
        return static_cast<std::size_t>(std::count_if(std::begin(seq), std::end(seq), pred_));
    }
};

// Implementation of default_if_empty operator (version without parameters).
template<typename = void>
class default_if_empty_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::const_value_type>
    {
        coveo::enumerable<typename seq_traits<Seq>::const_value_type> e;
        if (any_impl<>()(seq)) {
            e = coveo::enumerate_container(std::forward<Seq>(seq));
        } else {
            e = coveo::enumerate_one(typename seq_traits<Seq>::raw_value_type());
        }
        return e;
    }
};

// Implementation of default_if_empty operator (version with default value).
template<typename T>
class default_if_empty_impl_1
{
private:
    const T& obj_;  // Object to use to create default value if empty.

public:
    explicit default_if_empty_impl_1(const T& obj)
        : obj_(obj) { }

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::const_value_type>
    {
        coveo::enumerable<typename seq_traits<Seq>::const_value_type> e;
        if (any_impl<>()(seq)) {
            e = coveo::enumerate_container(std::forward<Seq>(seq));
        } else {
            e = coveo::enumerate_one(typename seq_traits<Seq>::raw_value_type(obj_));
        }
        return e;
    }
};

// Implementation of distinct operator.
template<typename Pred>
class distinct_impl
{
public:
    // Implementation of next delegate that filters duplicate elements
    template<typename Seq>
    class next_impl
    {
    private:
        // Type of iterator for the sequence
        typedef typename seq_traits<Seq>::iterator_type iterator_type;

        // Set storing pointers to seen elements
        typedef std::set<typename seq_traits<Seq>::const_pointer, deref_cmp<proxy_cmp<Pred>>>
                                                        seen_elements_set;

        // Info used to produce distinct elements. Shared among delegates.
        class distinct_info
        {
        private:
            Seq seq_;               // Sequence being iterated
            iterator_type iend_;    // Iterator pointing at end of sequence
            Pred pred_;             // Predicate ordering the elements

        public:
            distinct_info(Seq&& seq, Pred&& pred)
                : seq_(std::forward<Seq>(seq)),
                  iend_(std::end(seq_)),
                  pred_(std::forward<Pred>(pred)) { }

            // Cannot copy/move, stored in a shared_ptr
            distinct_info(const distinct_info&) = delete;
            distinct_info& operator=(const distinct_info&) = delete;

            iterator_type seq_begin() {
                return std::begin(seq_);
            }
            seen_elements_set init_seen_elements() {
                return seen_elements_set(deref_cmp<proxy_cmp<Pred>>(proxy_cmp<Pred>(pred_)));
            }

            // Returns next distinct element or nullptr when done
            auto get_next(iterator_type& icur, seen_elements_set& seen)
                -> typename seq_traits<Seq>::pointer
            {
                typename seq_traits<Seq>::pointer pobj = nullptr;
                for (; pobj == nullptr && icur != iend_; ++icur) {
                    typename seq_traits<Seq>::reference robjtmp = *icur;
                    auto pobjtmp = std::addressof(robjtmp);
                    if (seen.emplace(pobjtmp).second) {
                        // Not seen yet, return this element.
                        pobj = pobjtmp;
                    }
                }
                return pobj;
            }
        };
        typedef std::shared_ptr<distinct_info> distinct_info_sp;

        distinct_info_sp spinfo_;   // Shared info
        iterator_type icur_;        // Iterator pointing at current element
        seen_elements_set seen_;    // Set of seen elements

    public:
        next_impl(Seq&& seq, Pred&& pred)
            : spinfo_(std::make_shared<distinct_info>(std::forward<Seq>(seq), std::forward<Pred>(pred))),
              icur_(spinfo_->seq_begin()), seen_(spinfo_->init_seen_elements()) { }

        auto operator()() -> decltype(spinfo_->get_next(icur_, seen_)) {
            return spinfo_->get_next(icur_, seen_);
        }
    };

private:
    Pred pred_;     // Predicate used to compare elements
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    explicit distinct_impl(Pred&& pred)
        : pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    distinct_impl(const distinct_impl&) = delete;
    distinct_impl(distinct_impl&&) = default;
    distinct_impl& operator=(const distinct_impl&) = delete;
    distinct_impl& operator=(distinct_impl&&) = default;

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        // Note: if pred_ is not a ref, it is moved by the forward below
        // and becomes invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq>(std::forward<Seq>(seq), std::forward<Pred>(pred_));
    }
};

// Implementation of element_at operator.
template<typename = void>
class element_at_impl
{
private:
    std::size_t n_;     // Index of element to fetch.

private:
    // If we have random-access iterators, we can perform fast computations
    template<typename Seq>
    auto impl(Seq&& seq, std::random_access_iterator_tag) -> decltype(*std::begin(seq)) {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (static_cast<std::size_t>(iend - icur) <= n_) {
            throw_linq_out_of_range();
        }
        icur += n_;
        return *icur;
    }

    // Otherwise, we can only move by hand
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag) -> decltype(*std::begin(seq)) {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        for (std::size_t i = 0; i < n_ && icur != iend; ++i, ++icur) {
        }
        if (icur == iend) {
            throw_linq_out_of_range();
        }
        return *icur;
    }

public:
    explicit element_at_impl(std::size_t n)
        : n_(n) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of element_at_or_default operator.
template<typename = void>
class element_at_or_default_impl
{
private:
    std::size_t n_;     // Index of element to fetch.

private:
    // If we have random-access iterators, we can perform fast computations
    template<typename Seq>
    auto impl(Seq&& seq, std::random_access_iterator_tag) -> typename seq_traits<Seq>::raw_value_type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        return static_cast<std::size_t>(iend - icur) > n_ ? *(icur + n_)
                                                          : typename seq_traits<Seq>::raw_value_type();
    }

    // Otherwise, we can only move by hand
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag) -> typename seq_traits<Seq>::raw_value_type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        for (std::size_t i = 0; i < n_ && icur != iend; ++i, ++icur) {
        }
        return icur != iend ? *icur
                            : typename seq_traits<Seq>::raw_value_type();
    }

public:
    element_at_or_default_impl(std::size_t n)
        : n_(n) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of except operator.
template<typename Seq2, typename Pred>
class except_impl
{
public:
    // Implementation of next delegate that filters out
    // elements in the second sequence
    template<typename Seq1>
    class next_impl
    {
    private:
        // Vector of pointers to elements in the second sequence. Used to filter them out.
        typedef std::vector<typename seq_traits<Seq2>::const_pointer> elements_to_filter_v;

        // Type of iterator for the first sequence.
        typedef typename seq_traits<Seq1>::iterator_type first_iterator_type;

        // Bean storing info about elements to filter out. Shared among delegate instances.
        class filter_info
        {
        private:
            Seq1 seq1_;                         // Sequence of elements to scan and return.
            first_iterator_type iend1_;         // End of seq1_
            Seq2 seq2_;                         // Sequence of elements to filter out
            deref_cmp<Pred> pred_;              // Predicate used to compare
            elements_to_filter_v v_to_filter_;  // Elements to filter out. Late-initialized.
            bool init_;                         // Init flag for v_to_filter_.

        public:
            filter_info(Seq1&& seq1, Seq2&& seq2, Pred&& pred)
                : seq1_(std::forward<Seq1>(seq1)), iend1_(std::end(seq1_)),
                  seq2_(std::forward<Seq2>(seq2)), pred_(std::forward<Pred>(pred)),
                  v_to_filter_(), init_(false) { }

            // No move or copy, it's stored in a shared_ptr
            filter_info(const filter_info&) = delete;
            filter_info& operator=(const filter_info&) = delete;

            first_iterator_type first_begin() {
                return std::begin(seq1_);
            }

            bool filtered(typename seq_traits<Seq1>::pointer pobj) {
                if (!init_) {
                    // Init elements to filter on first call
                    try_reserve(v_to_filter_, seq2_);
                    auto icur2 = std::begin(seq2_);
                    auto iend2 = std::end(seq2_);
                    for (; icur2 != iend2; ++icur2) {
                        typename seq_traits<Seq2>::const_reference robjtmp = *icur2;
                        v_to_filter_.emplace_back(std::addressof(robjtmp));
                    }
                    std::sort(v_to_filter_.begin(), v_to_filter_.end(), pred_);
                    init_ = true;
                }
                return std::binary_search(v_to_filter_.cbegin(), v_to_filter_.cend(), pobj, pred_);
            }

            // Returns next non-filtered element or nullptr when done
            auto get_next(first_iterator_type& icur1) -> typename seq_traits<Seq1>::pointer {
                typename seq_traits<Seq1>::pointer pobj = nullptr;
                for (; pobj == nullptr && icur1 != iend1_; ++icur1) {
                    typename seq_traits<Seq1>::reference robjtmp = *icur1;
                    auto pobjtmp = std::addressof(robjtmp);
                    if (!filtered(pobjtmp)) {
                        pobj = pobjtmp;
                    }
                }
                return pobj;
            }
        };
        typedef std::shared_ptr<filter_info> filter_info_sp;

        filter_info_sp spfilter_;   // Bean containing filter info
        first_iterator_type icur_;  // Current position in first sequence

    public:
        next_impl(Seq1&& seq1, Seq2&& seq2, Pred&& pred)
            : spfilter_(std::make_shared<filter_info>(std::forward<Seq1>(seq1),
                                                      std::forward<Seq2>(seq2),
                                                      std::forward<Pred>(pred))),
              icur_(spfilter_->first_begin()) { }

        auto operator()() -> decltype(spfilter_->get_next(icur_)) {
            return spfilter_->get_next(icur_);
        }
    };

private:
    Seq2 seq2_;     // Sequence of elements to filter out
    Pred pred_;     // Predicate used to compare elements
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    except_impl(Seq2&& seq2, Pred&& pred)
        : seq2_(std::forward<Seq2>(seq2)), pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    except_impl(const except_impl&) = delete;
    except_impl(except_impl&&) = default;
    except_impl& operator=(const except_impl&) = delete;
    except_impl& operator=(except_impl&&) = default;

    template<typename Seq1>
    auto operator()(Seq1&& seq1)
        -> coveo::enumerable<typename seq_traits<Seq1>::value_type>
    {
        // Note: if seq2_ and/or pred_ are not refs, they will be moved
        // by the forward below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq1>(std::forward<Seq1>(seq1),
                               std::forward<Seq2>(seq2_),
                               std::forward<Pred>(pred_));
    }
};

// Implementation of first operator (version without parameters).
template<typename = void>
class first_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        auto icur = std::begin(seq);
        if (icur == std::end(seq)) {
            throw_linq_empty_sequence();
        }
        return *icur;
    }
};

// Implementation of first operator (version with predicate).
template<typename Pred>
class first_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy.

public:
    explicit first_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (icur == iend) {
            throw_linq_empty_sequence();
        }
        auto ifound = std::find_if(icur, iend, pred_);
        if (ifound == iend) {
            throw_linq_out_of_range();
        }
        return *ifound;
    }
};

// Implementation of first_or_default operator (version without parameters).
template<typename = void>
class first_or_default_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        auto icur = std::begin(seq);
        return icur != std::end(seq) ? *icur
                                     : typename seq_traits<Seq>::raw_value_type();
    }
};

// Implementation of first_or_default operator (version with predicate).
template<typename Pred>
class first_or_default_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy.

public:
    explicit first_or_default_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        auto iend = std::end(seq);
        auto ifound = std::find_if(std::begin(seq), iend, pred_);
        return ifound != iend ? *ifound
                              : typename seq_traits<Seq>::raw_value_type();
    }
};

// Implementation of group_by operator
template<typename KeySelector,
         typename ValueSelector,
         typename ResultSelector,
         typename Pred>
class group_by_impl
{
public:
    // Implementation of next delegate that returns group information
    template<typename Seq>
    class next_impl
    {
    public:
        // Key and value types returned by selectors.
        typedef decltype(std::declval<KeySelector>()(std::declval<typename seq_traits<Seq>::reference>()))      key;
        typedef decltype(std::declval<ValueSelector>()(std::declval<typename seq_traits<Seq>::reference>()))    value;

        // Vector of values sharing a common key.
        typedef std::vector<typename std::decay<value>::type>                   value_v;
        typedef decltype(coveo::enumerate_container(std::declval<value_v&&>())) values;

        // Map that stores keys and their corresponding values.
        typedef std::map<typename std::decay<key>::type, value_v, proxy_cmp<Pred>> values_by_key_m;

        // Result returned by result selector.
        typedef decltype(std::declval<ResultSelector>()(std::declval<key>(), std::declval<values>())) result;

        // Vector of results returned by this next delegate.
        typedef std::vector<typename std::decay<result>::type> result_v;

    private:
        // Bean storing group information. Shared among delegates in a shared_ptr.
        class groups_info
        {
        private:
            Seq seq_;                       // Sequence containing the elements
            KeySelector key_sel_;           // Returns keys for elements
            ValueSelector value_sel_;       // Returns values for elements
            ResultSelector result_sel_;     // Converts groups into end results
            Pred pred_;                     // Compares keys
            result_v results_;              // Vector of end results
            bool init_flag_;                // Whether results_ has been initialized

        public:
            groups_info(Seq&& seq, KeySelector&& key_sel, ValueSelector&& value_sel,
                        ResultSelector&& result_sel, Pred&& pred)
                : seq_(std::forward<Seq>(seq)),
                  key_sel_(std::forward<KeySelector>(key_sel)),
                  value_sel_(std::forward<ValueSelector>(value_sel)),
                  result_sel_(std::forward<ResultSelector>(result_sel)),
                  pred_(std::forward<Pred>(pred)),
                  results_(), init_flag_(false) { }

            // Not copyable/movable, stored in a shared_ptr
            groups_info(const groups_info&) = delete;
            groups_info& operator=(const groups_info&) = delete;

            const result_v& get_results() {
                // Init everything on first call
                if (!init_flag_) {
                    // First build map of groups
                    values_by_key_m groups{proxy_cmp<Pred>(pred_)};
                    for (auto&& obj : seq_) {
                        groups[key_sel_(obj)].emplace_back(value_sel_(obj));
                    }

                    // Now build vector of results
                    // Note that since we no longer need the map afterwards, we can actually move
                    // the vectors stored as map values into the results vector.
                    results_.reserve(groups.size());
                    for (const auto& group_pair : groups) {
                        results_.emplace_back(result_sel_(group_pair.first,
                                                          coveo::enumerate_container(std::move(group_pair.second))));
                    }

                    init_flag_ = true;
                }
                return results_;
            }
        };
        typedef std::shared_ptr<groups_info> groups_info_sp;

        groups_info_sp spgroups_;                   // Information about groups
        typename result_v::const_iterator icurr_;   // Iterator pointing at current result.
        typename result_v::const_iterator iendr_;   // Iterator pointing at end of result vector.
        bool init_flag_;                            // Whether icurr_ and iendr_ have been initialized

    public:
        next_impl(Seq&& seq, KeySelector&& key_sel, ValueSelector&& value_sel,
                  ResultSelector&& result_sel, Pred&& pred)
            : spgroups_(std::make_shared<groups_info>(std::forward<Seq>(seq),
                                                      std::forward<KeySelector>(key_sel),
                                                      std::forward<ValueSelector>(value_sel),
                                                      std::forward<ResultSelector>(result_sel),
                                                      std::forward<Pred>(pred))),
              icurr_(), iendr_(), init_flag_(false) { }

        auto operator()() -> typename seq_traits<result_v>::const_pointer {
            // Init iterators on first call
            if (!init_flag_) {
                const auto& results = spgroups_->get_results();
                icurr_ = std::begin(results);
                iendr_ = std::end(results);
                init_flag_ = true;
            }
            typename seq_traits<result_v>::const_pointer pobj = nullptr;
            if (icurr_ != iendr_) {
                typename seq_traits<result_v>::const_reference robj = *icurr_;
                pobj = std::addressof(robj);
                ++icurr_;
            }
            return pobj;
        }
    };

private:
    KeySelector key_sel_;       // Selector that provides keys for each element
    ValueSelector value_sel_;   // Selector that provides values for each element
    ResultSelector result_sel_; // Selector that converts each group into a final result
    Pred pred_;                 // Predicate used to compare keys
#ifdef _DEBUG
    bool applied_;              // Tracks operator application
#endif

public:
    group_by_impl(KeySelector&& key_sel, ValueSelector&& value_sel,
                  ResultSelector&& result_sel, Pred&& pred)
        : key_sel_(std::forward<KeySelector>(key_sel)),
          value_sel_(std::forward<ValueSelector>(value_sel)),
          result_sel_(std::forward<ResultSelector>(result_sel)),
          pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    group_by_impl(const group_by_impl&) = delete;
    group_by_impl(group_by_impl&&) = default;
    group_by_impl& operator=(const group_by_impl&) = delete;
    group_by_impl& operator=(group_by_impl&&) = default;

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<typename next_impl<Seq>::result_v>::const_value_type>
    {
        // Note: if members are not refs, they will be moved by the forwards
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq>(std::forward<Seq>(seq),
                              std::forward<KeySelector>(key_sel_),
                              std::forward<ValueSelector>(value_sel_),
                              std::forward<ResultSelector>(result_sel_),
                              std::forward<Pred>(pred_));
    }
};

// Implementation of group_join operator
template<typename InnerSeq,
         typename OuterKeySelector,
         typename InnerKeySelector,
         typename ResultSelector,
         typename Pred>
class group_join_impl
{
public:
    // Implementation of next delegate that returns final results
    template<typename OuterSeq>
    class next_impl
    {
    public:
        // Key returned by key selectors.
        typedef decltype(std::declval<OuterKeySelector>()(std::declval<typename seq_traits<OuterSeq>::reference>())) key;

        // Group of pointers to elements from the inner sequence that share a common key.
        typedef std::vector<typename seq_traits<InnerSeq>::pointer>                 inner_element_v;
        typedef coveo::enumerable<typename seq_traits<InnerSeq>::const_value_type>  inner_elements;
    
        // Result returned by result selector.
        typedef decltype(std::declval<ResultSelector>()(std::declval<typename seq_traits<OuterSeq>::reference>(),
                                                        std::declval<inner_elements>()))        result;

        // Vector of results returned by this next delegate.
        typedef std::vector<typename std::decay<result>::type> result_v;

    private:
        // Bean storing groups information. Shared among delegates in a shared_ptr.
        class groups_info
        {
        private:
            OuterSeq outer_seq_;                // Outer sequence.
            InnerSeq inner_seq_;                // Inner sequence from which to create groups.
            OuterKeySelector outer_key_sel_;    // Key selector for outer sequence.
            InnerKeySelector inner_key_sel_;    // Key selector for inner sequence.
            ResultSelector result_sel_;         // Selector converting groups into final results.
            Pred pred_;                         // Predicate used to compare keys.
            result_v results_;                  // Vector of final results.
            bool init_flag_;                    // Whether results_ has been initialized.

        public:
            groups_info(OuterSeq&& outer_seq, InnerSeq&& inner_seq,
                        OuterKeySelector&& outer_key_sel, InnerKeySelector&& inner_key_sel,
                        ResultSelector&& result_sel, Pred&& pred)
                : outer_seq_(std::forward<OuterSeq>(outer_seq)),
                  inner_seq_(std::forward<InnerSeq>(inner_seq)),
                  outer_key_sel_(std::forward<OuterKeySelector>(outer_key_sel)),
                  inner_key_sel_(std::forward<InnerKeySelector>(inner_key_sel)),
                  result_sel_(std::forward<ResultSelector>(result_sel)),
                  pred_(std::forward<Pred>(pred)),
                  results_(), init_flag_(false) { }

            // Not copyable/movable, stored in a shared_ptr
            groups_info(const groups_info&) = delete;
            groups_info& operator=(const groups_info&) = delete;

            const result_v& get_results() {
                // Init results on first call.
                if (!init_flag_) {
                    // Build map of groups of elements from inner sequence.
                    typedef std::map<typename std::decay<key>::type, inner_element_v, proxy_cmp<Pred>> groups_m;
                    groups_m keyed_inner_elems{proxy_cmp<Pred>(pred_)};
                    for (auto&& inner_elem : inner_seq_) {
                        typename seq_traits<InnerSeq>::reference robj = inner_elem;
                        keyed_inner_elems[inner_key_sel_(inner_elem)].emplace_back(std::addressof(robj));
                    }

                    // Iterate outer sequence and build final results by matching the elements with
                    // the groups we built earlier.
                    try_reserve(results_, outer_seq_);
                    auto iendki = keyed_inner_elems.end();
                    for (auto&& outer_elem : outer_seq_) {
                        key outer_key = outer_key_sel_(outer_elem);
                        auto icurki = keyed_inner_elems.find(outer_key);
                        inner_elements inner_elems;
                        if (icurki != iendki) {
                            std::size_t inner_size = icurki->second.size();
                            inner_elems = inner_elements(make_deref_next_impl(icurki->second),
                                                         [inner_size]() -> std::size_t { return inner_size; });
                        }
                        results_.emplace_back(result_sel_(outer_elem, inner_elems));
                    }

                    init_flag_ = true;
                }
                return results_;
            }
        };
        typedef std::shared_ptr<groups_info> groups_info_sp;

        groups_info_sp spgroups_;                   // Information about groups and results
        typename result_v::const_iterator icurr_;   // Iterator pointing at current result.
        typename result_v::const_iterator iendr_;   // Iterator pointing at end of result vector.
        bool init_flag_;                            // Whether icurr_/iendr_ have been initialized.

    public:
        next_impl(OuterSeq&& outer_seq, InnerSeq&& inner_seq,
                  OuterKeySelector&& outer_key_sel, InnerKeySelector&& inner_key_sel,
                  ResultSelector&& result_sel, Pred&& pred)
            : spgroups_(std::make_shared<groups_info>(std::forward<OuterSeq>(outer_seq),
                                                      std::forward<InnerSeq>(inner_seq),
                                                      std::forward<OuterKeySelector>(outer_key_sel),
                                                      std::forward<InnerKeySelector>(inner_key_sel),
                                                      std::forward<ResultSelector>(result_sel),
                                                      std::forward<Pred>(pred))),
              icurr_(), iendr_(), init_flag_(false) { }

        auto operator()() -> typename seq_traits<result_v>::const_pointer {
            // Init iterators on first call
            if (!init_flag_) {
                const auto& results = spgroups_->get_results();
                icurr_ = std::begin(results);
                iendr_ = std::end(results);
                init_flag_ = true;
            }
            typename seq_traits<result_v>::const_pointer pobj = nullptr;
            if (icurr_ != iendr_) {
                typename seq_traits<result_v>::const_reference robj = *icurr_;
                pobj = std::addressof(robj);
                ++icurr_;
            }
            return pobj;
        }
    };

private:
    InnerSeq inner_seq_;                // Inner sequence whose elements to group with outer elements with matching keys
    OuterKeySelector outer_key_sel_;    // Selector to get keys from elements in the outer sequence
    InnerKeySelector inner_key_sel_;    // Selector to get keys from elements in the inner sequence
    ResultSelector result_sel_;         // Selector to convert groups into final results
    Pred pred_;                         // Predicate used to compare keys
#ifdef _DEBUG
    bool applied_;                      // Tracks operator application
#endif

public:
    group_join_impl(InnerSeq&& inner_seq,
                    OuterKeySelector&& outer_key_sel,
                    InnerKeySelector&& inner_key_sel,
                    ResultSelector&& result_sel,
                    Pred&& pred)
        : inner_seq_(std::forward<InnerSeq>(inner_seq)),
          outer_key_sel_(std::forward<OuterKeySelector>(outer_key_sel)),
          inner_key_sel_(std::forward<InnerKeySelector>(inner_key_sel)),
          result_sel_(std::forward<ResultSelector>(result_sel)),
          pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    group_join_impl(const group_join_impl&) = delete;
    group_join_impl(group_join_impl&&) = default;
    group_join_impl& operator=(const group_join_impl&) = delete;
    group_join_impl& operator=(group_join_impl&&) = default;

    template<typename OuterSeq>
    auto operator()(OuterSeq&& outer_seq)
        -> coveo::enumerable<typename seq_traits<typename next_impl<OuterSeq>::result_v>::const_value_type>
    {
        // Note: if members are not refs, they will be moved by the forwards
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<OuterSeq>(std::forward<OuterSeq>(outer_seq),
                                   std::forward<InnerSeq>(inner_seq_),
                                   std::forward<OuterKeySelector>(outer_key_sel_),
                                   std::forward<InnerKeySelector>(inner_key_sel_),
                                   std::forward<ResultSelector>(result_sel_),
                                   std::forward<Pred>(pred_));
    }
};

// Implementation of intersect operator
template<typename Seq2, typename Pred>
class intersect_impl
{
public:
    // Implementation of next delegate that performs intersection
    template<typename Seq1>
    class next_impl
    {
    public:
        // Vector storing pointers to the elements.
        typedef std::vector<typename seq_traits<Seq2>::pointer> seq2_element_v;

        // Type of iterators for the sequences.
        typedef typename seq_traits<Seq1>::iterator_type    first_iterator_type;
        typedef typename seq_traits<Seq2>::iterator_type    second_iterator_type;

    private:
        // Information about intersection. Shared among delegates through smart_ptr.
        class intersect_info
        {
        private:
            Seq1 seq1_;                     // First sequence to intersect.
            first_iterator_type iend1_;     // Iterator pointing at end of first sequence.
            Seq2 seq2_;                     // Second sequence to intersect.
            deref_cmp<Pred> pred_;          // Predicate used to compare elements.
            seq2_element_v v_in_seq2_;      // Vector of elements from second sequence.
            bool init_flag_;                // Whether v_in_seq2_ has been initialized.

        public:
            intersect_info(Seq1&& seq1, Seq2&& seq2, Pred&& pred)
                : seq1_(std::forward<Seq1>(seq1)),
                  iend1_(std::end(seq1_)),
                  seq2_(std::forward<Seq2>(seq2)),
                  pred_(std::forward<Pred>(pred)),
                  v_in_seq2_(),
                  init_flag_(false) { }

            // Not copyable/movable, stored in a shared_ptr
            intersect_info(const intersect_info&) = delete;
            intersect_info& operator=(const intersect_info&) = delete;

            first_iterator_type first_begin() {
                return std::begin(seq1_);
            }

            bool is_in_seq2(typename seq_traits<Seq1>::pointer pobj) {
                // Build vector of second sequence on the first call.
                if (!init_flag_) {
                    // Add all elements from second sequence to a vector and sort it.
                    try_reserve(v_in_seq2_, seq2_);
                    auto icur2 = std::begin(seq2_);
                    auto iend2 = std::end(seq2_);
                    for (; icur2 != iend2; ++icur2) {
                        typename seq_traits<Seq2>::reference robjtmp = *icur2;
                        v_in_seq2_.emplace_back(std::addressof(robjtmp));
                    }
                    std::sort(v_in_seq2_.begin(), v_in_seq2_.end(), pred_);
                    init_flag_ = true;
                }
                return std::binary_search(v_in_seq2_.cbegin(), v_in_seq2_.cend(), pobj, pred_);
            }

            // Returns next element that is both in first and second sequence or nullptr when done
            auto get_next(first_iterator_type& icur1) -> typename seq_traits<Seq1>::pointer {
                typename seq_traits<Seq1>::pointer pobj = nullptr;
                for (; pobj == nullptr && icur1 != iend1_; ++icur1) {
                    typename seq_traits<Seq1>::reference robjtmp = *icur1;
                    auto pobjtmp = std::addressof(robjtmp);
                    if (is_in_seq2(pobjtmp)) {
                        pobj = pobjtmp;
                    }
                }
                return pobj;
            }
        };
        typedef std::shared_ptr<intersect_info> intersect_info_sp;

        intersect_info_sp spint_info_;      // Intersection information.
        first_iterator_type icur_;          // Current position in first sequence.

    public:
        next_impl(Seq1&& seq1, Seq2&& seq2, Pred&& pred)
            : spint_info_(std::make_shared<intersect_info>(std::forward<Seq1>(seq1),
                                                           std::forward<Seq2>(seq2),
                                                           std::forward<Pred>(pred))),
              icur_(spint_info_->first_begin()) { }

        auto operator()() -> decltype(spint_info_->get_next(icur_)) {
            return spint_info_->get_next(icur_);
        }
    };

private:
    Seq2 seq2_;     // Second sequence to intersect.
    Pred pred_;     // Predicate used to compare elements.
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    intersect_impl(Seq2&& seq2, Pred&& pred)
        : seq2_(std::forward<Seq2>(seq2)),
          pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    intersect_impl(const intersect_impl&) = delete;
    intersect_impl(intersect_impl&&) = default;
    intersect_impl& operator=(const intersect_impl&) = delete;
    intersect_impl& operator=(intersect_impl&&) = default;

    template<typename Seq1>
    auto operator()(Seq1&& seq1)
        -> coveo::enumerable<typename seq_traits<Seq1>::value_type>
    {
        // Note: if seq2_ and/or pred_ are not refs, they will be moved
        // by the forward below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq1>(std::forward<Seq1>(seq1),
                               std::forward<Seq2>(seq2_),
                               std::forward<Pred>(pred_));
    }
};

// Implementation of join operator
template<typename InnerSeq,
         typename OuterKeySelector,
         typename InnerKeySelector,
         typename ResultSelector,
         typename Pred>
class join_impl
{
public:
    // Implementation of next delegate that performs join
    template<typename OuterSeq>
    class next_impl
    {
    public:
        // Key returned by key selectors.
        typedef decltype(std::declval<OuterKeySelector>()(std::declval<typename seq_traits<OuterSeq>::reference>()))    key;

        // Group of pointers to elements from the inner sequence that share a common key.
        typedef std::vector<typename seq_traits<InnerSeq>::pointer> inner_element_v;
    
        // Result returned by result selector.
        typedef decltype(std::declval<ResultSelector>()(std::declval<typename seq_traits<OuterSeq>::reference>(),
                                                        std::declval<typename seq_traits<InnerSeq>::reference>()))      result;

        // Vector of results returned by this next delegate.
        typedef std::vector<typename std::decay<result>::type> result_v;

    private:
        // Bean storing join information. Shared among delegates in a shared_ptr.
        class join_info
        {
        private:
            OuterSeq outer_seq_;                // Outer sequence.
            InnerSeq inner_seq_;                // Inner sequence to join with outer sequence.
            OuterKeySelector outer_key_sel_;    // Key selector for outer sequence.
            InnerKeySelector inner_key_sel_;    // Key selector for inner sequence.
            ResultSelector result_sel_;         // Selector converting joined elements into final results.
            Pred pred_;                         // Predicate used to compare keys.
            result_v results_;                  // Vector of final results.
            bool init_flag_;                    // Whether results_ has been initialized.

        public:
            join_info(OuterSeq&& outer_seq, InnerSeq&& inner_seq,
                      OuterKeySelector&& outer_key_sel, InnerKeySelector&& inner_key_sel,
                      ResultSelector&& result_sel, Pred&& pred)
                : outer_seq_(std::forward<OuterSeq>(outer_seq)),
                  inner_seq_(std::forward<InnerSeq>(inner_seq)),
                  outer_key_sel_(std::forward<OuterKeySelector>(outer_key_sel)),
                  inner_key_sel_(std::forward<InnerKeySelector>(inner_key_sel)),
                  result_sel_(std::forward<ResultSelector>(result_sel)),
                  pred_(std::forward<Pred>(pred)),
                  results_(), init_flag_(false) { }

            // Not copyable/movable, stored in a shared_ptr
            join_info(const join_info&) = delete;
            join_info& operator=(const join_info&) = delete;

            const result_v& get_results() {
                // Init results on first call.
                if (!init_flag_) {
                    // Build map of groups of elements from inner sequence.
                    typedef std::map<typename std::decay<key>::type, inner_element_v, proxy_cmp<Pred>> groups_m;
                    groups_m keyed_inner_elems{proxy_cmp<Pred>(pred_)};
                    for (auto&& inner_elem : inner_seq_) {
                        typename seq_traits<InnerSeq>::reference robj = inner_elem;
                        keyed_inner_elems[inner_key_sel_(inner_elem)].emplace_back(std::addressof(robj));
                    }

                    // Iterate outer sequence and build final results by joining the elements with
                    // those in the groups we built earlier.
                    try_reserve(results_, inner_seq_);
                    auto iendki = keyed_inner_elems.end();
                    for (auto&& outer_elem : outer_seq_) {
                        key outer_key = outer_key_sel_(outer_elem);
                        auto icurki = keyed_inner_elems.find(outer_key);
                        if (icurki != iendki) {
                            for (auto* pinner_elem : icurki->second) {
                                results_.emplace_back(result_sel_(outer_elem, *pinner_elem));
                            }
                        }
                    }

                    init_flag_ = true;
                }
                return results_;
            }
        };
        typedef std::shared_ptr<join_info> join_info_sp;

        join_info_sp spjoin_info_;                  // Information about joined elements and results
        typename result_v::const_iterator icurr_;   // Iterator pointing at current result.
        typename result_v::const_iterator iendr_;   // Iterator pointing at end of result vector.
        bool init_flag_;                            // Whether icurr_/iendr_ have been initialized.

    public:
        next_impl(OuterSeq&& outer_seq, InnerSeq&& inner_seq,
                  OuterKeySelector&& outer_key_sel, InnerKeySelector&& inner_key_sel,
                  ResultSelector&& result_sel, Pred&& pred)
            : spjoin_info_(std::make_shared<join_info>(std::forward<OuterSeq>(outer_seq),
                                                       std::forward<InnerSeq>(inner_seq),
                                                       std::forward<OuterKeySelector>(outer_key_sel),
                                                       std::forward<InnerKeySelector>(inner_key_sel),
                                                       std::forward<ResultSelector>(result_sel),
                                                       std::forward<Pred>(pred))),
              icurr_(), iendr_(), init_flag_(false) { }

        auto operator()() -> typename seq_traits<result_v>::const_pointer {
            // Init iterators on first call
            if (!init_flag_) {
                const auto& results = spjoin_info_->get_results();
                icurr_ = std::begin(results);
                iendr_ = std::end(results);
                init_flag_ = true;
            }
            typename seq_traits<result_v>::const_pointer pobj = nullptr;
            if (icurr_ != iendr_) {
                typename seq_traits<result_v>::const_reference robj = *icurr_;
                pobj = std::addressof(robj);
                ++icurr_;
            }
            return pobj;
        }
    };

private:
    InnerSeq inner_seq_;                // Inner sequence to join.
    OuterKeySelector outer_key_sel_;    // Fetches keys for elements of outer sequence.
    InnerKeySelector inner_key_sel_;    // Fetches keys for elements of inner sequence.
    ResultSelector result_sel_;         // Creates results from joined elements.
    Pred pred_;                         // Predicate to compare keys.
#ifdef _DEBUG
    bool applied_;                      // Tracks operator application.
#endif

public:
    join_impl(InnerSeq&& inner_seq,
              OuterKeySelector&& outer_key_sel,
              InnerKeySelector&& inner_key_sel,
              ResultSelector&& result_sel,
              Pred&& pred)
        : inner_seq_(std::forward<InnerSeq>(inner_seq)),
          outer_key_sel_(std::forward<OuterKeySelector>(outer_key_sel)),
          inner_key_sel_(std::forward<InnerKeySelector>(inner_key_sel)),
          result_sel_(std::forward<ResultSelector>(result_sel)),
          pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    join_impl(const join_impl&) = delete;
    join_impl(join_impl&&) = default;
    join_impl& operator=(const join_impl&) = delete;
    join_impl& operator=(join_impl&&) = default;

    template<typename OuterSeq>
    auto operator()(OuterSeq&& outer_seq)
        -> coveo::enumerable<typename seq_traits<typename next_impl<OuterSeq>::result_v>::const_value_type>
    {
        // Note: if members are not refs, they will be moved by the forwards
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<OuterSeq>(std::forward<OuterSeq>(outer_seq),
                                   std::forward<InnerSeq>(inner_seq_),
                                   std::forward<OuterKeySelector>(outer_key_sel_),
                                   std::forward<InnerKeySelector>(inner_key_sel_),
                                   std::forward<ResultSelector>(result_sel_),
                                   std::forward<Pred>(pred_));
    }
};

// Implementation of last operator (version without argument)
template<typename = void>
class last_impl_0
{
private:
    // If we have bidi iterators, we can simply use rbegin
    template<typename Seq>
    auto impl(Seq&& seq, std::bidirectional_iterator_tag) -> decltype(*std::begin(seq)) {
        auto ricur = seq.rbegin();
        if (ricur == seq.rend()) {
            throw_linq_empty_sequence();
        }
        return *ricur;
    }

    // Otherwise we'll have to be creative
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag) -> decltype(*std::begin(seq)) {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (icur == iend) {
            throw_linq_empty_sequence();
        }
        decltype(icur) iprev;
        while (icur != iend) {
            iprev = icur;
            ++icur;
        }
        return *iprev;
    }

public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of last operator (version with predicate)
template<typename Pred>
class last_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy

private:
    // If we have bidi iterators, we can simply use rbegin
    template<typename Seq>
    auto impl(Seq&& seq, std::bidirectional_iterator_tag) -> decltype(*std::begin(seq)) {
        auto ricur = seq.rbegin();
        auto riend = seq.rend();
        if (ricur == riend) {
            throw_linq_empty_sequence();
        }
        auto rifound = std::find_if(ricur, riend, pred_);
        if (rifound == riend) {
            throw_linq_out_of_range();
        }
        return *rifound;
    }

    // Otherwise we'll have to be creative
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag) -> decltype(*std::begin(seq)) {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (icur == iend) {
            throw_linq_empty_sequence();
        }
        auto ifound = iend;
        while (icur != iend) {
            if (pred_(*icur)) {
                ifound = icur;
            }
            ++icur;
        }
        if (ifound == iend) {
            throw_linq_out_of_range();
        }
        return *ifound;
    }

public:
    explicit last_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of last_or_default operator (version without argument)
template<typename = void>
class last_or_default_impl_0
{
private:
    // If we have bidi iterators, we can simply use rbegin
    template<typename Seq>
    auto impl(Seq&& seq, std::bidirectional_iterator_tag) -> typename seq_traits<Seq>::raw_value_type {
        auto ricur = seq.rbegin();
        return ricur != seq.rend() ? *ricur
                                   : typename seq_traits<Seq>::raw_value_type();
    }

    // Otherwise we'll have to be creative
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag) -> typename seq_traits<Seq>::raw_value_type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        auto iprev = iend;
        while (icur != iend) {
            iprev = icur;
            ++icur;
        }
        return iprev != iend ? *iprev
                             : typename seq_traits<Seq>::raw_value_type();
    }

public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of last_or_default operator (version with predicate)
template<typename Pred>
class last_or_default_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy

private:
    // If we have bidi iterators, we can simply use rbegin
    template<typename Seq>
    auto impl(Seq&& seq, std::bidirectional_iterator_tag) -> typename seq_traits<Seq>::raw_value_type {
        auto riend = seq.rend();
        auto rifound = std::find_if(seq.rbegin(), riend, pred_);
        return rifound != riend ? *rifound
                                : typename seq_traits<Seq>::raw_value_type();
    }

    // Otherwise we'll have to be creative
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag) -> typename seq_traits<Seq>::raw_value_type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        auto ifound = iend;
        while (icur != iend) {
            if (pred_(*icur)) {
                ifound = icur;
            }
            ++icur;
        }
        return ifound != iend ? *ifound
                              : typename seq_traits<Seq>::raw_value_type();
    }

public:
    explicit last_or_default_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of max operator (version without parameters).
template<typename = void>
class max_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        auto iend = std::end(seq);
        auto imax = std::max_element(std::begin(seq), iend);
        if (imax == iend) {
            throw_linq_empty_sequence();
        }
        return *imax;
    }
};

// Implementation of max operator (version with selector).
template<typename Selector>
class max_impl_1
{
private:
    const Selector& sel_;   // Selector used to fetch values from sequence elements.

public:
    explicit max_impl_1(const Selector& sel)
        : sel_(sel) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename std::decay<decltype(sel_(*std::begin(seq)))>::type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (icur == iend) {
            throw_linq_empty_sequence();
        }
        auto max_val = sel_(*icur);
        while (++icur != iend) {
            max_val = std::max(max_val, sel_(*icur));
        }
        return max_val;
    }
};

// Implementation of min operator (version without parameters).
template<typename = void>
class min_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        auto iend = std::end(seq);
        auto imin = std::min_element(std::begin(seq), iend);
        if (imin == iend) {
            throw_linq_empty_sequence();
        }
        return *imin;
    }
};

// Implementation of min operator (version with selector).
template<typename Selector>
class min_impl_1
{
private:
    const Selector& sel_;   // Selector used to fetch values from sequence elements.

public:
    explicit min_impl_1(const Selector& sel)
        : sel_(sel) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename std::decay<decltype(sel_(*std::begin(seq)))>::type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (icur == iend) {
            throw_linq_empty_sequence();
        }
        auto min_val = sel_(*icur);
        while (++icur != iend) {
            min_val = std::min(min_val, sel_(*icur));
        }
        return min_val;
    }
};

// Implementation of a comparator using KeySelector and Pred.
// Has an operator() that returns an int representing the comparison
// between two objects (negative values means left is before right, etc.)
template<typename KeySelector,
         typename Pred,
         bool Descending,
         int _LessValue = Descending ? 1 : -1>
class order_by_comparator
{
private:
    KeySelector key_sel_;   // Key selector used to fetch keys from elements.
    Pred pred_;             // Predicate used to compare keys.

public:
    order_by_comparator(KeySelector&& key_sel, Pred&& pred)
        : key_sel_(std::forward<KeySelector>(key_sel)), pred_(std::forward<Pred>(pred)) { }

    // Cannot copy/move, stored in a unique_ptr
    order_by_comparator(const order_by_comparator&) = delete;
    order_by_comparator& operator=(const order_by_comparator&) = delete;

    // Compares two values, returning relative position of the two in an ordered sequence.
    template<typename T1, typename T2>
    int operator()(T1&& left, T2&& right) const {
        decltype(key_sel_(left)) leftk = key_sel_(left);
        decltype(key_sel_(right)) rightk = key_sel_(right);
        int cmp;
        if (pred_(leftk, rightk)) {
            cmp = _LessValue;
        } else if (pred_(rightk, leftk)) {
            cmp = -_LessValue;
        } else {
            // Keys are equal.
            cmp = 0;
        }
        return cmp;
    }
};

// Implementation of a comparator that calls two comparators in order.
// If the first comparator returns 0, calls the second comparator.
template<typename Cmp1, typename Cmp2>
class dual_order_by_comparator
{
private:
    std::unique_ptr<Cmp1> upcmp1_;  // First comparator called.
    std::unique_ptr<Cmp2> upcmp2_;  // Second comparator called.

public:
    dual_order_by_comparator(std::unique_ptr<Cmp1>&& upcmp1, std::unique_ptr<Cmp2>&& upcmp2)
        : upcmp1_(std::move(upcmp1)), upcmp2_(std::move(upcmp2)) { }

    // Cannot copy/move, stored in a unique_ptr
    dual_order_by_comparator(const dual_order_by_comparator&) = delete;
    dual_order_by_comparator& operator=(const dual_order_by_comparator&) = delete;

    // Compares two values by calling first then second comparator.
    template<typename T1, typename T2>
    int operator()(T1&& left, T2&& right) const {
        int cmp = (*upcmp1_)(left, right);
        if (cmp == 0) {
            cmp = (*upcmp2_)(left, right);
        }
        return cmp;
    }
};

// Forward declaration to declare friendship
template<typename Cmp> class order_by_impl;

// Implementation of order_by/then_by operators.
// This is the operator with a sequence.
template<typename Seq, typename Cmp>
class order_by_impl_with_seq
{
    // We need this friendship so that it can "steal" our internals.
    template<typename> friend class order_by_impl;

private:
    Seq seq_;                                                           // Sequence we're ordering.
    std::unique_ptr<Cmp> upcmp_;                                        // Comparator used to order a sequence.
    coveo::enumerable<typename seq_traits<Seq>::value_type> enum_;      // Enumerator of ordered elements.
    bool init_flag_;                                                    // Whether enum_ has been initialized.

    // Called to initialize enum_ before using it.
    void init() {
        std::vector<typename seq_traits<Seq>::pointer> ordered;
        try_reserve(ordered, seq_);
        for (auto&& elem : seq_) {
            typename seq_traits<Seq>::reference robj = elem;
            ordered.push_back(std::addressof(robj));
        }
        std::stable_sort(ordered.begin(),
                         ordered.end(),
                         [this](typename seq_traits<Seq>::pointer pleft,
                                typename seq_traits<Seq>::pointer pright) {
            return (*upcmp_)(*pleft, *pright) < 0;
        });
        std::size_t num_ordered = ordered.size();
        enum_ = decltype(enum_)(make_deref_next_impl(std::move(ordered)),
                                [num_ordered]() -> std::size_t { return num_ordered; });
        init_flag_ = true;
    }

public:
    // Type of iterators used for the ordered sequence.
    typedef typename coveo::enumerable<typename seq_traits<Seq>::value_type>::iterator          iterator;
    typedef typename coveo::enumerable<typename seq_traits<Seq>::value_type>::const_iterator    const_iterator;

    // Constructor called by the impl without sequence.
    order_by_impl_with_seq(Seq&& seq, std::unique_ptr<Cmp>&& upcmp)
        : seq_(std::forward<Seq>(seq)), upcmp_(std::move(upcmp)), enum_(), init_flag_(false) { }

    // Movable but not copyable
    order_by_impl_with_seq(const order_by_impl_with_seq&) = delete;
    order_by_impl_with_seq(order_by_impl_with_seq&&) = default;
    order_by_impl_with_seq& operator=(const order_by_impl_with_seq&) = delete;
    order_by_impl_with_seq& operator=(order_by_impl_with_seq&&) = default;

    // Support for ordered sequence.
    iterator begin() const {
        if (!init_flag_) {
            const_cast<order_by_impl_with_seq<Seq, Cmp>*>(this)->init();
        }
        return enum_.begin();
    }
    iterator end() const {
        if (!init_flag_) {
            const_cast<order_by_impl_with_seq<Seq, Cmp>*>(this)->init();
        }
        return enum_.end();
    }

    // Support for sequence size (a bit like the enumerable API)
    bool has_fast_size() const {
        if (!init_flag_) {
            const_cast<order_by_impl_with_seq<Seq, Cmp>*>(this)->init();
        }
        return enum_.has_fast_size();
    }
    std::size_t size() const {
        if (!init_flag_) {
            const_cast<order_by_impl_with_seq<Seq, Cmp>*>(this)->init();
        }
        return enum_.size();
    }
};

// Implementation of order_by/then_by operators.
// This is the "initial" operator without a sequence, as returned by helper methods.
template<typename Cmp>
class order_by_impl
{
private:
    std::unique_ptr<Cmp> upcmp_;    // Comparator used to order a sequence.

public:
    explicit order_by_impl(std::unique_ptr<Cmp>&& upcmp)
        : upcmp_(std::move(upcmp)) { }

    // Movable by not copyable
    order_by_impl(const order_by_impl&) = delete;
    order_by_impl(order_by_impl&&) = default;
    order_by_impl& operator=(const order_by_impl&) = delete;
    order_by_impl& operator=(order_by_impl&&) = default;

    // When applied to a sequence, produces a different object.
    template<typename Seq>
    auto operator()(Seq&& seq) -> order_by_impl_with_seq<Seq, Cmp> {
        // Cannot apply twice.
        assert(upcmp_ != nullptr);

        return order_by_impl_with_seq<Seq, Cmp>(std::forward<Seq>(seq), std::move(upcmp_));
    }

    // When applied to an impl with sequence, merges the two and chains the comparators.
    template<typename ImplSeq, typename ImplCmp>
    auto operator()(order_by_impl_with_seq<ImplSeq, ImplCmp>&& impl)
        -> order_by_impl_with_seq<ImplSeq, dual_order_by_comparator<ImplCmp, Cmp>>
    {
        typedef dual_order_by_comparator<ImplCmp, Cmp> dual_comparator;
        auto new_upcmp = detail::make_unique<dual_comparator>(std::move(impl.upcmp_), std::move(upcmp_));
        return order_by_impl_with_seq<ImplSeq, dual_comparator>(std::forward<ImplSeq>(impl.seq_), std::move(new_upcmp));
    }
};

// Implementation of reverse operator
template<typename = void>
class reverse_impl
{
private:
    // next impl when we can use reverse iterators
    template<typename Seq>
    class next_impl_fast
    {
    public:
        // Iterator used to go backward in sequence.
        typedef typename std::decay<decltype(std::declval<Seq>().rbegin())>::type reverse_iterator;

    private:
        struct reverse_info {
            Seq seq_;                   // Sequence we're iterating.
            reverse_iterator irend_;    // End of reversed seq_.

            explicit reverse_info(Seq&& seq)
                : seq_(std::forward<Seq>(seq)),
                  irend_(seq_.rend()) { }

            // Cannot copy/move, stored in a shared_ptr.
            reverse_info(const reverse_info&) = delete;
            reverse_info& operator=(const reverse_info&) = delete;
        };
        typedef std::shared_ptr<reverse_info> reverse_info_sp;

        reverse_info_sp spinfo_;    // Shared info with sequence.
        reverse_iterator ircur_;    // Current point in reverse sequence.

    public:
        explicit next_impl_fast(Seq&& seq)
            : spinfo_(std::make_shared<reverse_info>(std::forward<Seq>(seq))),
              ircur_(spinfo_->seq_.rbegin()) { }

        auto operator()() -> typename seq_traits<Seq>::pointer {
            typename seq_traits<Seq>::pointer pobj = nullptr;
            if (ircur_ != spinfo_->irend_) {
                typename seq_traits<Seq>::reference robj = *ircur_;
                pobj = std::addressof(robj);
                ++ircur_;
            }
            return pobj;
        }
    };

    // next impl when we don't have reverse iterators natively
    template<typename Seq>
    class next_impl_slow
    {
    public:
        // Vector storing pointers of elements from sequence.
        typedef std::vector<typename seq_traits<Seq>::pointer>  pelems_v;
        typedef typename pelems_v::reverse_iterator             pelems_v_reverse_iterator;

    private:
        struct reverse_info {
            Seq seq_;                           // Sequence containing the elements.
            pelems_v vpelems_;                  // Vector of pointers to the elements.
            pelems_v_reverse_iterator virend_;  // Iterator pointing at end of vpelems_.

            explicit reverse_info(Seq&& seq)
                : seq_(std::forward<Seq>(seq)),
                  vpelems_(), virend_()
            {
                // Build vector of pointers to sequence elements.
                try_reserve(vpelems_, seq_);
                for (auto&& elem : seq_) {
                    typename seq_traits<Seq>::reference robj = elem;
                    vpelems_.push_back(std::addressof(robj));
                }

                // Init end iterator.
                virend_ = vpelems_.rend();
            }

            // Cannot copy/move, stored in a shared_ptr.
            reverse_info(const reverse_info&) = delete;
            reverse_info& operator=(const reverse_info&) = delete;
        };
        typedef std::shared_ptr<reverse_info> reverse_info_sp;

        reverse_info_sp spinfo_;            // Bean storing sequence and vector of pointers.
        pelems_v_reverse_iterator vircur_;  // Iterator pointing at current pointer to element.

    public:
        explicit next_impl_slow(Seq&& seq)
            : spinfo_(std::make_shared<reverse_info>(std::forward<Seq>(seq))),
              vircur_(spinfo_->vpelems_.rbegin()) { }

        auto operator()() -> typename seq_traits<Seq>::pointer {
            typename seq_traits<Seq>::pointer pobj = nullptr;
            if (vircur_ != spinfo_->virend_) {
                pobj = *vircur_;
                ++vircur_;
            }
            return pobj;
        }

        // To be able to build a proper size delegate
        std::size_t size() const {
            return spinfo_->vpelems_.size();
        }
    };

    // If we have bidi iterators, we can simply use rbegin
    template<typename Seq>
    auto impl(Seq&& seq, std::bidirectional_iterator_tag)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        auto siz = try_get_size_delegate<typename seq_traits<Seq>::value_type>(seq);
        return coveo::enumerable<typename seq_traits<Seq>::value_type>(next_impl_fast<Seq>(std::forward<Seq>(seq)),
                                                                       siz);
    }

    // Otherwise we'll have to be creative
    template<typename Seq>
    auto impl(Seq&& seq, std::input_iterator_tag)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        next_impl_slow<Seq> next_impl(std::forward<Seq>(seq));
        std::size_t size = next_impl.size();
        auto siz = [size]() -> std::size_t { return size; };
        return coveo::enumerable<typename seq_traits<Seq>::value_type>(std::move(next_impl),
                                                                       siz);
    }

public:
    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        return impl(std::forward<Seq>(seq),
                    typename std::iterator_traits<typename seq_traits<Seq>::iterator_type>::iterator_category());
    }
};

// Implementation of select and select_with_index operators
template<typename Selector>
class select_impl
{
public:
    // Next delegate implementation for select operator
    template<typename Seq, typename CU, typename RU>
    class next_impl
    {
    private:
        // Iterator used by the sequence.
        typedef typename seq_traits<Seq>::iterator_type iterator_type;

        // Containers storing transformed elements.
        typedef std::vector<RU>                     transformed_v;
        typedef std::forward_list<RU>               transformed_l;
        typedef typename transformed_l::iterator    transformed_l_iterator;

        // Bean storing info about elements. Shared among delegates.
        struct select_info {
            Seq seq_;                       // Sequence being transformed.
            iterator_type icur_;            // Iterator pointing at current element in seq_.
            iterator_type iend_;            // Iterator pointing at end of seq_.
            Selector sel_;                  // Selector transforming the elements.
            transformed_v vtransformed_;    // Vector of transformed elements.
            transformed_l ltransformed_;    // List of transformed elements.
            transformed_l_iterator llast_;  // Iterator pointing to last element in ltransformed_ (before end()).
            std::size_t lsize_;             // Number of elements in ltransformed_.
            bool use_vector_;               // Whether we use ltransformed_ (false) or vtransformed_ (true).

            select_info(Seq&& seq, Selector&& sel)
                : seq_(std::forward<Seq>(seq)),
                  icur_(std::begin(seq_)),
                  iend_(std::end(seq_)),
                  sel_(std::forward<Selector>(sel)),
                  vtransformed_(),
                  ltransformed_(),
                  llast_(ltransformed_.before_begin()),
                  lsize_(0),
                  use_vector_(try_reserve(vtransformed_, seq_)) { }

            // Cannot copy/move, stored in a shared_ptr
            select_info(const select_info&) = delete;
            select_info& operator=(const select_info&) = delete;

            auto get_next(std::size_t& vncur, transformed_l_iterator& licur) -> CU* {
                CU* pnext = nullptr;
                if (use_vector_) {
                    while (vtransformed_.size() <= vncur && icur_ != iend_) {
                        vtransformed_.emplace_back(sel_(*icur_, vtransformed_.size()));
                        ++icur_;
                    }
                    if (vtransformed_.size() > vncur) {
                        pnext = std::addressof(vtransformed_[vncur++]);
                    }
                } else {
                    while (licur == llast_ && icur_ != iend_) {
                        llast_ = ltransformed_.emplace_after(llast_, sel_(*icur_, lsize_++));
                        ++icur_;
                    }
                    if (licur != llast_) {
                        pnext = std::addressof(*++licur);
                    }
                }
                return pnext;
            }
        };
        typedef std::shared_ptr<select_info> select_info_sp;

        select_info_sp spinfo_;         // Shared information about elements.
        std::size_t vncur_;             // Index of current element (if in vector).
        transformed_l_iterator licur_;  // Iterator pointing at current element (if in list).

    public:
        next_impl(Seq&& seq, Selector&& sel)
            : spinfo_(std::make_shared<select_info>(std::forward<Seq>(seq),
                                                    std::forward<Selector>(sel))),
              vncur_(0),
              licur_(spinfo_->ltransformed_.before_begin()) { }

        auto operator()() -> CU* {
            return spinfo_->get_next(vncur_, licur_);
        }
    };

private:
    Selector sel_;  // Selector used to transform elements.
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    explicit select_impl(Selector&& sel)
        : sel_(std::forward<Selector>(sel))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    template<typename Seq,
             typename _SelectorRes = decltype(std::declval<Selector>()(std::declval<typename seq_traits<Seq>::reference>(), std::declval<std::size_t>())),
             typename _CU = typename coveo::detail::seq_element_traits<_SelectorRes>::const_value_type,
             typename _RU = typename coveo::detail::seq_element_traits<_SelectorRes>::raw_value_type>
    auto operator()(Seq&& seq) -> coveo::enumerable<_CU> {
        // Note: if sel_ is not a ref, it will be moved by the forward
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        auto siz = try_get_size_delegate<_CU>(seq);
        return coveo::enumerable<_CU>(next_impl<Seq, _CU, _RU>(std::forward<Seq>(seq), std::forward<Selector>(sel_)),
                                      siz);
    }
};

// Implementation of select_many and select_many_with_index operators
template<typename Selector>
class select_many_impl
{
public:
    // Next delegate implementation for select_many operator
    template<typename Seq, typename CU, typename RU>
    class next_impl
    {
    private:
        // Iterator used by the sequence.
        typedef typename seq_traits<Seq>::iterator_type iterator_type;

        // List storing transformed elements.
        typedef std::forward_list<RU>               transformed_l;
        typedef typename transformed_l::iterator    transformed_l_iterator;

        // Bean storing info about elements. Shared among delegates.
        struct select_info {
            Seq seq_;                       // Sequence being transformed.
            iterator_type icur_;            // Iterator pointing at current element in seq_.
            std::size_t idx_;               // Index of current element in seq_.
            iterator_type iend_;            // Iterator pointing at end of seq_.
            Selector sel_;                  // Selector transforming the elements.
            transformed_l ltransformed_;    // List of transformed elements.
            transformed_l_iterator llast_;  // Iterator pointing to last element in ltransformed_ (before end()).

            select_info(Seq&& seq, Selector&& sel)
                : seq_(std::forward<Seq>(seq)),
                  icur_(std::begin(seq_)),
                  idx_(0),
                  iend_(std::end(seq_)),
                  sel_(std::forward<Selector>(sel)),
                  ltransformed_(),
                  llast_(ltransformed_.before_begin()) { }

            // Cannot copy/move, stored in a shared_ptr
            select_info(const select_info&) = delete;
            select_info& operator=(const select_info&) = delete;

            auto get_next(transformed_l_iterator& licur) -> CU* {
                CU* pnext = nullptr;
                while (licur == llast_ && icur_ != iend_) {
                    auto new_elements = sel_(*icur_, idx_++);
                    llast_ = ltransformed_.insert_after(llast_, std::begin(new_elements), std::end(new_elements));
                    ++icur_;
                }
                if (licur != llast_) {
                    pnext = std::addressof(*++licur);
                }
                return pnext;
            }
        };
        typedef std::shared_ptr<select_info> select_info_sp;

        select_info_sp spinfo_;         // Shared information about elements.
        transformed_l_iterator licur_;  // Iterator pointing at current element.

    public:
        next_impl(Seq&& seq, Selector&& sel)
            : spinfo_(std::make_shared<select_info>(std::forward<Seq>(seq),
                                                    std::forward<Selector>(sel))),
              licur_(spinfo_->ltransformed_.before_begin()) { }

        auto operator()() -> CU* {
            return spinfo_->get_next(licur_);
        }
    };

private:
    Selector sel_;  // Selector used to transform elements.
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    explicit select_many_impl(Selector&& sel)
        : sel_(std::forward<Selector>(sel))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    template<typename Seq,
             typename _SelectorSeqRes = decltype(std::declval<Selector>()(std::declval<typename seq_traits<Seq>::const_reference>(), std::declval<std::size_t>())),
             typename _CU = typename seq_traits<_SelectorSeqRes>::const_value_type,
             typename _RU = typename seq_traits<_SelectorSeqRes>::raw_value_type>
    auto operator()(Seq&& seq) -> coveo::enumerable<_CU> {
        // Note: if sel_ is not a ref, it will be moved by the forward
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq, _CU, _RU>(std::forward<Seq>(seq), std::forward<Selector>(sel_));
    }
};

// Implementation of sequence_equal operator (version with sequence only).
template<typename Seq2>
class sequence_equal_impl_1
{
private:
    const Seq2& seq2_;  // Second sequence to compare.

public:
    explicit sequence_equal_impl_1(const Seq2& seq2)
        : seq2_(seq2) { }

    template<typename Seq1>
    auto operator()(Seq1&& seq1) -> bool {
        auto icur1 = std::begin(seq1);
        auto iend1 = std::end(seq1);
        auto icur2 = std::begin(seq2_);
        auto iend2 = std::end(seq2_);
        bool is_equal = true;
        for (; is_equal && icur1 != iend1 && icur2 != iend2; ++icur1, ++icur2) {
            is_equal = *icur1 == *icur2;
        }
        return is_equal && icur1 == iend1 && icur2 == iend2;
    }
};

// Implementation of sequence_equal operator (version with predicate).
template<typename Seq2, typename Pred>
class sequence_equal_impl_2
{
private:
    const Seq2& seq2_;  // Second sequence to compare.
    const Pred& pred_;  // Equality predicate.

public:
    sequence_equal_impl_2(const Seq2& seq2, const Pred& pred)
        : seq2_(seq2), pred_(pred) { }

    template<typename Seq1>
    auto operator()(Seq1&& seq1) -> bool {
        auto icur1 = std::begin(seq1);
        auto iend1 = std::end(seq1);
        auto icur2 = std::begin(seq2_);
        auto iend2 = std::end(seq2_);
        bool is_equal = true;
        for (; is_equal && icur1 != iend1 && icur2 != iend2; ++icur1, ++icur2) {
            is_equal = pred_(*icur1, *icur2);
        }
        return is_equal && icur1 == iend1 && icur2 == iend2;
    }
};

// Implementation of single operator (version without parameters).
template<typename = void>
class single_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        auto ifirst = std::begin(seq);
        auto iend = std::end(seq);
        if (ifirst == iend) {
            throw_linq_empty_sequence();
        }
        auto inext = ifirst;
        ++inext;
        if (inext != iend) {
            throw_linq_out_of_range();
        }
        return *ifirst;
    }
};

// Implementation of single operator (version with predicate).
template<typename Pred>
class single_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy.

public:
    explicit single_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> decltype(*std::begin(seq)) {
        auto ibeg = std::begin(seq);
        auto iend = std::end(seq);
        if (ibeg == iend) {
            throw_linq_empty_sequence();
        }
        auto ifound = std::find_if(ibeg, iend, pred_);
        if (ifound == iend) {
            throw_linq_out_of_range();
        }
        auto inext = ifound;
        ++inext;
        auto ifoundagain = std::find_if(inext, iend, pred_);
        if (ifoundagain != iend) {
            throw_linq_out_of_range();
        }
        return *ifound;
    }
};

// Implementation of single_or_default operator (version without parameters).
template<typename = void>
class single_or_default_impl_0
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        auto icur = std::begin(seq);
        auto iend = std::end(seq);
        if (icur != iend) {
            auto inext = icur;
            ++inext;
            if (inext != iend) {
                icur = iend;
            }
        }
        return icur != iend ? *icur
                            : typename seq_traits<Seq>::raw_value_type();
    }
};

// Implementation of single_or_default operator (version with predicate).
template<typename Pred>
class single_or_default_impl_1
{
private:
    const Pred& pred_;  // Predicate to satisfy.

public:
    explicit single_or_default_impl_1(const Pred& pred)
        : pred_(pred) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename seq_traits<Seq>::raw_value_type {
        auto iend = std::end(seq);
        auto ifound = std::find_if(std::begin(seq), iend, pred_);
        if (ifound != iend) {
            auto inext = ifound;
            ++inext;
            auto ifoundagain = std::find_if(inext, iend, pred_);
            if (ifoundagain != iend) {
                ifound = iend;
            }
        }
        return ifound != iend ? *ifound
                              : typename seq_traits<Seq>::raw_value_type();
    }
};

// Predicate implementation used by skip operator
template<typename = void>
class skip_n_pred
{
private:
    std::size_t n_; // Number of elements to skip.

public:
    explicit skip_n_pred(std::size_t n)
        : n_(n) { }

    template<typename T>
    auto operator()(T&&, std::size_t idx) -> bool {
        return idx < n_;
    }
};

// Implementation of skip/skip_while/skip_while_with_index operators
template<typename Pred>
class skip_impl
{
public:
    // Next delegate implementation for skip operators
    template<typename Seq>
    class next_impl
    {
    public:
        // Iterator for the sequence type.
        typedef typename seq_traits<Seq>::iterator_type iterator_type;

        // Bean containing info to share among delegates
        struct skip_info {
            Seq seq_;               // Sequence to skip elements from
            iterator_type iend_;    // Iterator pointing at end of sequence
            Pred pred_;             // Predicate to satisfy to skip elements

            skip_info(Seq&& seq, Pred&& pred)
                : seq_(std::forward<Seq>(seq)),
                  iend_(std::end(seq_)),
                  pred_(std::forward<Pred>(pred)) { }

            // Cannot copy/move, stored in a shared_ptr
            skip_info(const skip_info&) = delete;
            skip_info& operator=(const skip_info&) = delete;
        };
        typedef std::shared_ptr<skip_info> skip_info_sp;

    private:
        skip_info_sp spinfo_;   // Pointer to shared info
        iterator_type icur_;    // Iterator pointing at current element
        bool init_flag_;        // Whether icur_ has been initialized

    public:
        next_impl(Seq&& seq, Pred&& pred)
            : spinfo_(std::make_shared<skip_info>(std::forward<Seq>(seq),
                                                  std::forward<Pred>(pred))),
              icur_(), init_flag_(false) { }

        auto operator()() -> typename seq_traits<Seq>::pointer {
            // Init starting point on first call
            if (!init_flag_) {
                icur_ = std::begin(spinfo_->seq_);
                std::size_t n = 0;
                while (icur_ != spinfo_->iend_ && spinfo_->pred_(*icur_, n++)) {
                    ++icur_;
                }
                init_flag_ = true;
            }
            typename seq_traits<Seq>::pointer pobj = nullptr;
            if (icur_ != spinfo_->iend_) {
                typename seq_traits<Seq>::reference robj = *icur_;
                pobj = std::addressof(robj);
                ++icur_;
            }
            return pobj;
        }
    };

private:
    Pred pred_;     // Predicate to satisfy to skip.
    std::size_t n_; // How many items to skip, if known (otherwise -1).
#ifdef _DEBUG
    bool applied_;  // Tracks operator application.
#endif

public:
    skip_impl(Pred&& pred, std::size_t n)
        : pred_(std::forward<Pred>(pred)),
          n_(n)
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        // Note: if pred_ is not a ref, it will be moved by the forward
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        typename coveo::enumerable<typename seq_traits<Seq>::value_type>::size_delegate siz;
        if (n_ != static_cast<std::size_t>(-1)) {
            auto seq_siz = try_get_size_delegate<typename seq_traits<Seq>::value_type>(seq);
            if (seq_siz != nullptr) {
                std::size_t seq_size = seq_siz();
                std::size_t size = seq_size > n_ ? seq_size - n_ : 0;
                siz = [size]() -> std::size_t { return size; };
            }
        }
        return coveo::enumerable<typename seq_traits<Seq>::value_type>(next_impl<Seq>(std::forward<Seq>(seq),
                                                                                      std::forward<Pred>(pred_)),
                                                                       siz);
    }
};

// Implementation of sum operator.
template<typename F>
class sum_impl
{
private:
    const F& num_f_;    // Function to fetch values from sequence elements.

public:
    explicit sum_impl(const F& num_f)
        : num_f_(num_f) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> typename std::decay<decltype(num_f_(*std::begin(seq)))>::type {
        auto it = std::begin(seq);
        auto end = std::end(seq);
        if (it == end) {
            throw_linq_empty_sequence();
        }
        auto total = num_f_(*it);
        for (++it; it != end; ++it) {
            total += num_f_(*it);
        }
        return total;
    }
};

// Implementation of take/take_while/take_while_with_index operators
template<typename Pred>
class take_impl
{
public:
    // Next delegate implementation for take operators
    template<typename Seq>
    class next_impl
    {
    public:
        // Iterator for the sequence type.
        typedef typename seq_traits<Seq>::iterator_type iterator_type;

        // Bean containing info to share among delegates
        struct take_info {
            Seq seq_;               // Sequence to take elements from
            iterator_type iend_;    // Iterator pointing at end of sequence
            Pred pred_;             // Predicate to satisfy to take elements

            take_info(Seq&& seq, Pred&& pred)
                : seq_(std::forward<Seq>(seq)),
                  iend_(std::end(seq_)),
                  pred_(std::forward<Pred>(pred)) { }

            // Cannot copy/move, stored in a shared_ptr
            take_info(const take_info&) = delete;
            take_info& operator=(const take_info&) = delete;
        };
        typedef std::shared_ptr<take_info> take_info_sp;

    private:
        take_info_sp spinfo_;       // Pointer to shared info
        iterator_type icur_;        // Iterator pointing at current element
        std::size_t n_;             // Index of current element
        bool done_;                 // Whether we're done taking elements

    public:
        next_impl(Seq&& seq, Pred&& pred)
            : spinfo_(std::make_shared<take_info>(std::forward<Seq>(seq),
                                                  std::forward<Pred>(pred))),
              icur_(std::begin(spinfo_->seq_)), n_(0), done_(false) { }

        auto operator()() -> typename seq_traits<Seq>::pointer {
            typename seq_traits<Seq>::pointer pobj = nullptr;
            if (!done_) {
                if (icur_ != spinfo_->iend_ && spinfo_->pred_(*icur_, n_++)) {
                    typename seq_traits<Seq>::reference robj = *icur_;
                    pobj = std::addressof(robj);
                    ++icur_;
                } else {
                    done_ = true;
                }
            }
            return pobj;
        }
    };

private:
    Pred pred_;     // Predicate to satisfy to skip.
    std::size_t n_; // How many items to take, if known (otherwise -1).
#ifdef _DEBUG
    bool applied_;  // Tracks operator application.
#endif

public:
    take_impl(Pred&& pred, std::size_t n)
        : pred_(std::forward<Pred>(pred)),
          n_(n)
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        // Note: if pred_ is not a ref, it will be moved by the forward
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        typename coveo::enumerable<typename seq_traits<Seq>::value_type>::size_delegate siz;
        if (n_ != static_cast<std::size_t>(-1)) {
            auto seq_siz = try_get_size_delegate<typename seq_traits<Seq>::value_type>(seq);
            if (seq_siz != nullptr) {
                std::size_t size = std::min(n_, seq_siz());
                siz = [size]() -> std::size_t { return size; };
            }
        }
        return coveo::enumerable<typename seq_traits<Seq>::value_type>(next_impl<Seq>(std::forward<Seq>(seq),
                                                                                      std::forward<Pred>(pred_)),
                                                                       siz);
    }
};

// Implementation of to operator.
template<typename Container>
class to_impl
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> Container {
        return Container(std::begin(std::forward<Seq>(seq)), std::end(std::forward<Seq>(seq)));
    }
};

// Implementation of to_vector operator.
template<typename = void>
class to_vector_impl
{
public:
    template<typename Seq>
    auto operator()(Seq&& seq) -> std::vector<typename seq_traits<Seq>::raw_value_type> {
        std::vector<typename seq_traits<Seq>::raw_value_type> v;
        try_reserve(v, seq);
        v.insert(v.end(), std::begin(std::forward<Seq>(seq)), std::end(std::forward<Seq>(seq)));
        return v;
    }
};

// Implementation of to_associative operator (version with key selector only).
template<typename Container, typename KeySelector>
class to_associative_impl_1
{
private:
    const KeySelector& key_sel_;    // Selector to fetch keys for sequence elements.

public:
    explicit to_associative_impl_1(const KeySelector& key_sel)
        : key_sel_(key_sel) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> Container {
        Container c;
        for (auto&& elem : seq) {
            auto key = key_sel_(elem);
            auto emplace_res = c.emplace(key, elem);
            if (!emplace_res.second) {
                emplace_res.first->second = elem;
            }
        }
        return c;
    }
};

// Implementation of to_associative operator (version with key and element selectors).
template<typename Container, typename KeySelector, typename ElementSelector>
class to_associative_impl_2
{
private:
    const KeySelector& key_sel_;        // Selector to fetch keys for sequence elements.
    const ElementSelector& elem_sel_;   // Selector to fetch values for sequence elements.

public:
    to_associative_impl_2(const KeySelector& key_sel, const ElementSelector& elem_sel)
        : key_sel_(key_sel), elem_sel_(elem_sel) { }

    template<typename Seq>
    auto operator()(Seq&& seq) -> Container {
        Container c;
        for (auto&& elem : seq) {
            auto key = key_sel_(elem);
            auto mapped = elem_sel_(elem);
            auto emplace_res = c.emplace(key, mapped);
            if (!emplace_res.second) {
                emplace_res.first->second = mapped;
            }
        }
        return c;
    }
};

// Implementation of to_map operator (version with key selector only).
template<typename KeySelector>
class to_map_impl_1
{
private:
    const KeySelector& key_sel_;    // Selector to fetch keys for sequence elements.

public:
    explicit to_map_impl_1(const KeySelector& key_sel)
        : key_sel_(key_sel) { }

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> std::map<typename std::decay<decltype(key_sel_(*std::begin(seq)))>::type,
                    typename seq_traits<Seq>::raw_value_type>
    {
        std::map<typename std::decay<decltype(key_sel_(*std::begin(seq)))>::type,
                 typename seq_traits<Seq>::raw_value_type> m;
        for (auto&& elem : seq) {
            auto key = key_sel_(elem);
            auto emplace_res = m.emplace(key, elem);
            if (!emplace_res.second) {
                emplace_res.first->second = elem;
            }
        }
        return m;
    }
};

// Implementation of to_map operator (version with key and element selectors).
template<typename KeySelector, typename ElementSelector>
class to_map_impl_2
{
private:
    const KeySelector& key_sel_;        // Selector to fetch keys for sequence elements.
    const ElementSelector& elem_sel_;   // Selector to fetch values for sequence elements.

public:
    to_map_impl_2(const KeySelector& key_sel, const ElementSelector& elem_sel)
        : key_sel_(key_sel), elem_sel_(elem_sel) { }

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> std::map<typename std::decay<decltype(key_sel_(*std::begin(seq)))>::type,
                    typename std::decay<decltype(elem_sel_(*std::begin(seq)))>::type>
    {
        std::map<typename std::decay<decltype(key_sel_(*std::begin(seq)))>::type,
                 typename std::decay<decltype(elem_sel_(*std::begin(seq)))>::type> m;
        for (auto&& elem : seq) {
            auto key = key_sel_(elem);
            auto mapped = elem_sel_(elem);
            auto emplace_res = m.emplace(key, mapped);
            if (!emplace_res.second) {
                emplace_res.first->second = mapped;
            }
        }
        return m;
    }
};

// Implementation of union_with operator.
template<typename Seq2, typename Pred>
class union_impl
{
public:
    // Implementation of next delegate that filters duplicate elements
    template<typename Seq1>
    class next_impl
    {
    public:
        // Type of element returned by this next delegate. The elements will be const
        // if at least one sequence is const.
        typedef typename std::conditional<std::is_const<typename seq_traits<Seq1>::value_type>::value ||
                                          std::is_const<typename seq_traits<Seq2>::value_type>::value,
                                          typename seq_traits<Seq1>::const_value_type,
                                          typename seq_traits<Seq1>::value_type>::type  enum_type;
        typedef typename coveo::detail::seq_element_traits<enum_type>::pointer          enum_ptr_type;
        typedef typename coveo::detail::seq_element_traits<enum_type>::reference        enum_ref_type;

    private:
        // Type of iterator for the sequences
        typedef typename seq_traits<Seq1>::iterator_type    first_iterator_type;
        typedef typename seq_traits<Seq2>::iterator_type    second_iterator_type;

        // Set storing seen elements
        typedef std::set<enum_ptr_type, deref_cmp<proxy_cmp<Pred>>> seen_elements_set;

        // Info used to produce distinct elements. Shared among delegates.
        class union_info
        {
        private:
            Seq1 seq1_;                     // First sequence being iterated
            first_iterator_type iend1_;     // Iterator pointing at end of first sequence
            Seq2 seq2_;                     // Second sequence being iterated
            second_iterator_type iend2_;    // Iterator pointing at end of second sequence
            Pred pred_;                     // Predicate ordering the elements

        public:
            union_info(Seq1&& seq1, Seq2&& seq2, Pred&& pred)
                : seq1_(std::forward<Seq1>(seq1)),
                  iend1_(std::end(seq1_)),
                  seq2_(std::forward<Seq2>(seq2)),
                  iend2_(std::end(seq2_)),
                  pred_(std::forward<Pred>(pred)) { }

            // Cannot copy/move, stored in a shared_ptr
            union_info(const union_info&) = delete;
            union_info& operator=(const union_info&) = delete;

            first_iterator_type first_begin() {
                return std::begin(seq1_);
            }
            second_iterator_type second_begin() {
                return std::begin(seq2_);
            }
            seen_elements_set init_seen_elements() {
                return seen_elements_set(deref_cmp<proxy_cmp<Pred>>(proxy_cmp<Pred>(pred_)));
            }

            // Returns next distinct element in either sequence or nullptr when done
            auto get_next(first_iterator_type& icur1, second_iterator_type& icur2, seen_elements_set& seen) -> enum_ptr_type {
                // First look for an element in first sequence
                enum_ptr_type pobj = nullptr;
                for (; pobj == nullptr && icur1 != iend1_; ++icur1) {
                    enum_ref_type robjtmp = *icur1;
                    auto pobjtmp = std::addressof(robjtmp);
                    if (seen.emplace(pobjtmp).second) {
                        // Not seen yet, return this element.
                        pobj = pobjtmp;
                    }
                }

                // If we did not find an element in first sequence, try in second sequence
                for (; pobj == nullptr && icur2 != iend2_; ++icur2) {
                    enum_ref_type robjtmp = *icur2;
                    auto pobjtmp = std::addressof(robjtmp);
                    if (seen.emplace(pobjtmp).second) {
                        pobj = pobjtmp;
                    }
                }
                return pobj;
            }
        };
        typedef std::shared_ptr<union_info> union_info_sp;

        union_info_sp spinfo_;          // Shared info
        first_iterator_type icur1_;     // Iterator pointing at current element in first sequence
        second_iterator_type icur2_;    // Iterator pointing at current element in second sequence
        seen_elements_set seen_;        // Set of seen elements

    public:
        next_impl(Seq1&& seq1, Seq2&& seq2, Pred&& pred)
            : spinfo_(std::make_shared<union_info>(std::forward<Seq1>(seq1),
                                                   std::forward<Seq2>(seq2),
                                                   std::forward<Pred>(pred))),
              icur1_(spinfo_->first_begin()),
              icur2_(spinfo_->second_begin()),
              seen_(spinfo_->init_seen_elements()) { }

        auto operator()() -> decltype(spinfo_->get_next(icur1_, icur2_, seen_)) {
            return spinfo_->get_next(icur1_, icur2_, seen_);
        }
    };

private:
    Seq2 seq2_;     // Second sequence to union.
    Pred pred_;     // Predicate used to compare elements
#ifdef _DEBUG
    bool applied_;  // Tracks operator application
#endif

public:
    explicit union_impl(Seq2&& seq2, Pred&& pred)
        : seq2_(std::forward<Seq2>(seq2)),
          pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    union_impl(const union_impl&) = delete;
    union_impl(union_impl&&) = default;
    union_impl& operator=(const union_impl&) = delete;
    union_impl& operator=(union_impl&&) = default;

    template<typename Seq1>
    auto operator()(Seq1&& seq1)
        -> coveo::enumerable<typename next_impl<Seq1>::enum_type>
    {
        // Note: if the members above are not refs, they are moved by the
        // forward below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq1>(std::forward<Seq1>(seq1),
                               std::forward<Seq2>(seq2_),
                               std::forward<Pred>(pred_));
    }
};

// Implementation of where/where_with_index operators.
template<typename Pred>
class where_impl
{
public:
    // Implementation of next delegate for where/where_with_index operators.
    template<typename Seq>
    class next_impl
    {
    public:
        // Iterator for the sequence.
        typedef typename seq_traits<Seq>::iterator_type iterator_type;

    private:
        // Bean storing info shared among delegates.
        struct where_info {
            Seq seq_;               // Sequence being iterated.
            iterator_type iend_;    // Iterator pointing at end of seq_
            Pred pred_;             // Predicate to satisfy

            where_info(Seq&& seq, Pred&& pred)
                : seq_(std::forward<Seq>(seq)),
                  iend_(std::end(seq_)),
                  pred_(std::forward<Pred>(pred)) { }

            // Cannot copy/move, stored in a shared_ptr
            where_info(const where_info&) = delete;
            where_info& operator=(const where_info&) = delete;
        };
        typedef std::shared_ptr<where_info> where_info_sp;

        where_info_sp spinfo_;      // Shared info containing sequence and predicate.
        iterator_type icur_;        // Pointer at current element.
        std::size_t idx_;           // Index of current element.

    public:
        next_impl(Seq&& seq, Pred&& pred)
            : spinfo_(std::make_shared<where_info>(std::forward<Seq>(seq),
                                                   std::forward<Pred>(pred))),
              icur_(std::begin(spinfo_->seq_)), idx_(0) { }

        auto operator()() -> typename seq_traits<Seq>::pointer {
            typename seq_traits<Seq>::pointer pobj = nullptr;
            for (; pobj == nullptr && icur_ != spinfo_->iend_; ++icur_, ++idx_) {
                typename seq_traits<Seq>::reference robjtmp = *icur_;
                auto pobjtmp = std::addressof(robjtmp);
                if (spinfo_->pred_(*pobjtmp, idx_)) {
                    // This element satistifies the predicate, return it.
                    pobj = pobjtmp;
                }
            }
            return pobj;
        }
    };

private:
    Pred pred_;     // Predicate to satisfy.
#ifdef _DEBUG
    bool applied_;  // Tracks operator application.
#endif

public:
    explicit where_impl(Pred&& pred)
        : pred_(std::forward<Pred>(pred))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    where_impl(const where_impl&) = delete;
    where_impl(where_impl&&) = default;
    where_impl& operator=(const where_impl&) = delete;
    where_impl& operator=(where_impl&&) = default;

    template<typename Seq>
    auto operator()(Seq&& seq)
        -> coveo::enumerable<typename seq_traits<Seq>::value_type>
    {
        // Note: if pred_ is not a ref, it will be moved by the forward
        // below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        return next_impl<Seq>(std::forward<Seq>(seq), std::forward<Pred>(pred_));
    }
};

// Implementation of zip operator.
template<typename Seq2, typename ResultSelector>
class zip_impl
{
public:
    // Implementation of next delegate for this operator.
    template<typename Seq1, typename CU, typename RU>
    class next_impl
    {
    public:
        // Iterator types for the sequences.
        typedef typename seq_traits<Seq1>::iterator_type    first_iterator_type;
        typedef typename seq_traits<Seq2>::iterator_type    second_iterator_type;

        // Containers storing zipped elements.
        typedef std::vector<RU>             zipped_v;
        typedef std::forward_list<RU>       zipped_l;
        typedef typename zipped_l::iterator zipped_l_iterator;

    private:
        // Bean storing info shared among delegates.
        struct zip_info {
            Seq1 seq1_;                     // First sequence to zip.
            first_iterator_type icur1_;     // Iterator pointing at current element of seq1_.
            first_iterator_type iend1_;     // Iterator pointing at end of seq1_.
            Seq2 seq2_;                     // Second sequence to zip.
            second_iterator_type icur2_;    // Iterator pointing at current element of seq2_.
            second_iterator_type iend2_;    // Iterator pointing at end of seq2_.
            ResultSelector result_sel_;     // Selector producing the results.
            zipped_v vzipped_;              // Vector of zipped elements.
            zipped_l lzipped_;              // List of zipped elements.
            zipped_l_iterator llast_;       // Iterator pointing to last element in lzipped_ (before end()).
            bool use_vector_;               // Whether we use lzipped_ (false) or vzipped_ (true).

            zip_info(Seq1&& seq1, Seq2&& seq2, ResultSelector&& result_sel,
                     const typename coveo::enumerable<CU>::size_delegate& siz)
                : seq1_(std::forward<Seq1>(seq1)),
                  icur1_(std::begin(seq1_)),
                  iend1_(std::end(seq1_)),
                  seq2_(std::forward<Seq2>(seq2)),
                  icur2_(std::begin(seq2_)),
                  iend2_(std::end(seq2_)),
                  result_sel_(std::forward<ResultSelector>(result_sel)),
                  vzipped_(),
                  lzipped_(),
                  llast_(lzipped_.before_begin()),
                  use_vector_(siz != nullptr)
            {
                if (siz != nullptr) {
                    vzipped_.reserve(siz());
                }
            }

            // Cannot copy/move, stored in a shared_ptr
            zip_info(const zip_info&) = delete;
            zip_info& operator=(const zip_info&) = delete;

            auto get_next(std::size_t& vncur, zipped_l_iterator& licur) -> CU* {
                CU* pnext = nullptr;
                if (use_vector_) {
                    while (vzipped_.size() <= vncur && icur1_ != iend1_ && icur2_ != iend2_) {
                        vzipped_.emplace_back(result_sel_(*icur1_, *icur2_));
                        ++icur1_;
                        ++icur2_;
                    }
                    if (vzipped_.size() > vncur) {
                        pnext = std::addressof(vzipped_[vncur++]);
                    }
                } else {
                    while (licur == llast_ && icur1_ != iend1_ && icur2_ != iend2_) {
                        llast_ = lzipped_.emplace_after(llast_, result_sel_(*icur1_, *icur2_));
                        ++icur1_;
                        ++icur2_;
                    }
                    if (licur != llast_) {
                        pnext = std::addressof(*++licur);
                    }
                }
                return pnext;
            }
        };
        typedef std::shared_ptr<zip_info>   zip_info_sp;

        zip_info_sp spinfo_;            // Bean containing shared info.
        std::size_t vncur_;             // Index of current element (if in vector).
        zipped_l_iterator licur_;       // Iterator pointing at current element (if in list).

    public:
        next_impl(Seq1&& seq1, Seq2&& seq2, ResultSelector&& result_sel,
                  const typename coveo::enumerable<CU>::size_delegate& siz)
            : spinfo_(std::make_shared<zip_info>(std::forward<Seq1>(seq1),
                                                 std::forward<Seq2>(seq2),
                                                 std::forward<ResultSelector>(result_sel),
                                                 siz)),
              vncur_(0),
              licur_(spinfo_->lzipped_.before_begin()) { }

        auto operator()() -> CU* {
            return spinfo_->get_next(vncur_, licur_);
        }
    };

private:
    Seq2 seq2_;                     // Second sequence to zip.
    ResultSelector result_sel_;     // Selector producing the results.
#ifdef _DEBUG
    bool applied_;                  // Tracks operator application.
#endif

public:
    zip_impl(Seq2&& seq2, ResultSelector&& result_sel)
        : seq2_(std::forward<Seq2>(seq2)),
          result_sel_(std::forward<ResultSelector>(result_sel))
#ifdef _DEBUG
        , applied_(false)
#endif
    { }

    // Movable but not copyable
    zip_impl(const zip_impl&) = delete;
    zip_impl(zip_impl&&) = default;
    zip_impl& operator=(const zip_impl&) = delete;
    zip_impl& operator=(zip_impl&&) = default;

    template<typename Seq1,
             typename _SelectorRes = decltype(std::declval<ResultSelector>()(std::declval<typename seq_traits<Seq1>::reference>(),
                                                                             std::declval<typename seq_traits<Seq2>::reference>())),
             typename _CU = typename coveo::detail::seq_element_traits<_SelectorRes>::const_value_type,
             typename _RU = typename coveo::detail::seq_element_traits<_SelectorRes>::raw_value_type>
    auto operator()(Seq1&& seq1) -> coveo::enumerable<_CU> {
        // Note: if the members above are not refs, they are moved by the
        // forward below and become invalid; this cannot be called twice.
#ifdef _DEBUG
        assert(!applied_);
        applied_ = true;
#endif
        auto siz1 = try_get_size_delegate<_CU>(seq1);
        auto siz2 = try_get_size_delegate<_CU>(seq2_);
        typename coveo::enumerable<_CU>::size_delegate siz;
        if (siz1 != nullptr && siz2 != nullptr) {
            std::size_t min_size = std::min(siz1(), siz2());
            siz = [min_size]() -> std::size_t { return min_size; };
        }
        return coveo::enumerable<_CU>(next_impl<Seq1, _CU, _RU>(std::forward<Seq1>(seq1),
                                                                std::forward<Seq2>(seq2_),
                                                                std::forward<ResultSelector>(result_sel_),
                                                                siz),
                                      siz);
    }
};

} // detail
} // linq
} // coveo

#endif // COVEO_LINQ_DETAIL_H
