#pragma once

#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

#include "dang-utils/global.h"
#include "dang-utils/utils.h"

namespace dang::utils {

// Despite having constexpr everywhere, TreeNode cannot be used constexpr yet.
// This will change in C++23, which allows std::unique_ptr in constexpr contexts.
// Therefore constexpr is already added everywhere in anticipation for C++23.

template <typename TData, bool v_leaf_data = false, bool v_optional_data = false>
class Tree;

template <typename TData, bool v_leaf_data = false>
using TreeOptionalData = Tree<TData, v_leaf_data, true>;

template <typename TData, bool v_optional_data = false>
using TreeLeafData = Tree<TData, true, v_optional_data>;

template <typename TData>
using TreeOptionalLeafData = Tree<TData, true, true>;

template <typename TData, bool v_leaf_data = false, bool v_optional_data = false>
class TreeNode;

template <typename TData, bool v_leaf_data = false>
using TreeNodeOptionalData = TreeNode<TData, v_leaf_data, true>;

template <typename TData, bool v_optional_data = false>
using TreeNodeLeafData = TreeNode<TData, true, v_optional_data>;

template <typename TData>
using TreeNodeOptionalLeafData = TreeNode<TData, true, true>;

namespace tree_tag {

struct Empty {};
static constexpr Empty empty;

struct DefaultValue {};
static constexpr DefaultValue default_value;

struct SupportChildren {};
static constexpr SupportChildren support_children;

} // namespace tree_tag

namespace detail {

template <typename TData, bool v_leaf_data, bool v_optional_data>
using TreeChildren = std::vector<Tree<TData, v_leaf_data, v_optional_data>>;

// --- TreeStorage

template <typename TData>
struct TreeStorage {
    TData data;
    TreeChildren<TData, false, false> children;
};

template <typename TData>
struct TreeStorageOptionalData {
    std::optional<TData> data;
    TreeChildren<TData, false, true> children;
};

template <typename TData>
using TreeStorageLeafData = std::variant<TData, TreeChildren<TData, true, false>>;

template <typename TData>
using TreeStorageOptionalLeafData = std::variant<std::monostate, TData, TreeChildren<TData, true, true>>;

template <typename TData, bool v_leaf_data = false, bool v_optional_data = false>
struct tree_storage;

template <typename TData>
struct tree_storage<TData, false, false> : std::type_identity<TreeStorage<TData>> {};

template <typename TData>
struct tree_storage<TData, false, true> : std::type_identity<TreeStorageOptionalData<TData>> {};

template <typename TData>
struct tree_storage<TData, true, false> : std::type_identity<TreeStorageLeafData<TData>> {};

template <typename TData>
struct tree_storage<TData, true, true> : std::type_identity<TreeStorageOptionalLeafData<TData>> {};

template <typename TData, bool v_leaf_data, bool v_optional_data>
using tree_storage_t = typename tree_storage<TData, v_leaf_data, v_optional_data>::type;

// --- TreeInit

template <typename TTreeInit>
std::vector<typename TTreeInit::Tree> createChildren(std::initializer_list<TTreeInit> children)
{
    std::vector<typename TTreeInit::Tree> result;
    (void)children;
    // TODO: Can't do this, because of missing parent... This needs to be done on TreeNodes directly.
    return result;
}

template <typename TData, bool v_leaf_data, bool v_optional_data>
class TreeInit;

template <typename TData>
class TreeInit<TData, false, false> {
public:
    using Data = TData;
    using Tree = Tree<TData>;
    using Storage = TreeStorage<Data>;
    using Children = std::initializer_list<TreeInit>;

    TreeInit(tree_tag::DefaultValue) {}

    TreeInit(const Data& data, Children children = {})
        : data_(data)
        , children_(children)
    {}

    TreeInit(Data&& data, Children children = {})
        : data_(std::move(data))
        , children_(children)
    {}

    TreeInit(Children children)
        : children_(children)
    {}

    operator Storage() const& { return {data_, createChildren(children_)}; }
    operator Storage() && { return {std::move(data_), createChildren(children_)}; }

private:
    Data data_ = {};
    Children children_;
};

template <typename TData>
class TreeInit<TData, false, true> {
public:
    using Data = TData;
    using Tree = TreeOptionalData<Data>;
    using Storage = TreeStorageOptionalData<Data>;
    using Children = std::initializer_list<TreeInit>;

    TreeInit(tree_tag::Empty) {}

    TreeInit(tree_tag::DefaultValue)
        : data_(Data())
    {}

    TreeInit(const Data& data, Children children = {})
        : data_(data)
        , children_(children)
    {}

    TreeInit(Data&& data, Children children = {})
        : data_(std::move(data))
        , children_(children)
    {}

    TreeInit(const std::optional<Data>& data, Children children = {})
        : data_(data)
        , children_(children)
    {}

    TreeInit(std::optional<Data>&& data, Children children = {})
        : data_(std::move(data))
        , children_(children)
    {}

    TreeInit(Children children)
        : children_(children)
    {}

    operator Storage() const& { return {data_, createChildren(children_)}; }
    operator Storage() && { return {std::move(data_), createChildren(children_)}; }

private:
    std::optional<Data> data_;
    Children children_;
};

template <typename TData>
class TreeInit<TData, true, false> {
public:
    using Data = TData;
    using Tree = TreeLeafData<Data>;
    using Storage = TreeStorageLeafData<Data>;
    using Children = std::initializer_list<TreeInit>;

    TreeInit(tree_tag::DefaultValue) {}

    TreeInit(tree_tag::SupportChildren)
        : data_or_children_(Children())
    {}

    TreeInit(const Data& data)
        : data_or_children_(data)
    {}

    TreeInit(Data&& data)
        : data_or_children_(std::move(data))
    {}

    TreeInit(Children children)
        : data_or_children_(children)
    {}

    TreeInit(const std::variant<Data, Children>& data_or_children)
        : data_or_children_(data_or_children)
    {}

    TreeInit(std::variant<Data, Children>&& data_or_children)
        : data_or_children_(std::move(data_or_children))
    {}

    operator Storage() const&
    {
        return std::visit(Overloaded{[](const Data& data) -> Storage { return data; },
                                     [](Children children) -> Storage { return createChildren<TreeInit>(children); }},
                          data_or_children_);
    }

    operator Storage() &&
    {
        return std::visit(Overloaded{[](Data&& data) -> Storage { return std::move(data); },
                                     [](Children children) -> Storage { return createChildren<TreeInit>(children); }},
                          std::move(data_or_children_));
    }

private:
    std::variant<Data, Children> data_or_children_;
};

template <typename TData>
class TreeInit<TData, true, true> {
public:
    using Data = TData;
    using Tree = TreeOptionalLeafData<Data>;
    using Storage = TreeStorageOptionalLeafData<Data>;
    using Children = std::initializer_list<TreeInit>;

    TreeInit(tree_tag::Empty) {}

    TreeInit(tree_tag::DefaultValue)
        : data_or_children_(Data())
    {}

    TreeInit(tree_tag::SupportChildren)
        : data_or_children_(Children())
    {}

    TreeInit(const Data& data)
        : data_or_children_(data)
    {}

    TreeInit(Data&& data)
        : data_or_children_(std::move(data))
    {}

    TreeInit(Children children)
        : data_or_children_(children)
    {}

    TreeInit(const std::variant<std::monostate, Data, Children>& data_or_children)
        : data_or_children_(data_or_children)
    {}

    TreeInit(std::variant<std::monostate, Data, Children>&& data_or_children)
        : data_or_children_(std::move(data_or_children))
    {}

    operator Storage() const&
    {
        return std::visit(Overloaded{[](std::monostate) -> Storage { return std::monostate{}; },
                                     [](const Data& data) -> Storage { return data; },
                                     [](Children children) -> Storage { return createChildren<TreeInit>(children); }},
                          data_or_children_);
    }

    operator Storage() &&
    {
        return std::visit(Overloaded{[](std::monostate) -> Storage { return std::monostate{}; },
                                     [](Data&& data) -> Storage { return std::move(data); },
                                     [](Children children) -> Storage { return createChildren<TreeInit>(children); }},
                          std::move(data_or_children_));
    }

private:
    std::variant<std::monostate, Data, Children> data_or_children_;
};

// --- TreeNodeIterator

template <typename TBaseIterator>
class TreeNodeIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type =
        copy_const_t<typename TBaseIterator::value_type::Node, std::remove_pointer_t<typename TBaseIterator::pointer>>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    TreeNodeIterator() = default;

    TreeNodeIterator& operator++()
    {
        base_iterator_++;
        return *this;
    }

    TreeNodeIterator operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    TreeNodeIterator& operator--()
    {
        base_iterator_--;
        return *this;
    }

    TreeNodeIterator operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    TreeNodeIterator& operator+=(difference_type diff)
    {
        base_iterator_ += diff;
        return *this;
    }

    TreeNodeIterator& operator-=(difference_type diff)
    {
        base_iterator_ -= diff;
        return *this;
    }

    TreeNodeIterator operator+(difference_type diff) const { return base_iterator_ + diff; }
    friend TreeNodeIterator operator+(difference_type diff, TreeNodeIterator iter) { return iter + diff; }
    TreeNodeIterator operator-(difference_type diff) const { return base_iterator_ - diff; }

    difference_type operator-(TreeNodeIterator other) const { return base_iterator_ - other.base_iterator_; }

    reference operator[](difference_type diff) const { return base_iterator_ + diff; }

    bool operator==(TreeNodeIterator other) const { return base_iterator_ == other.base_iterator_; }
    bool operator!=(TreeNodeIterator other) const { return !(*this == other); }
    bool operator<(TreeNodeIterator other) const { return base_iterator_ < other.base_iterator_; }
    bool operator<=(TreeNodeIterator other) const { return !(*this > other); }
    bool operator>(TreeNodeIterator other) const { return other < *this; }
    bool operator>=(TreeNodeIterator other) const { return !(*this < other); }

    reference operator*() const { return **base_iterator_; }
    pointer operator->() const { return &*base_iterator_; }

private:
    template <typename, bool, bool>
    friend class TreeChildrenRef;

    TreeNodeIterator(TBaseIterator base_iterator)
        : base_iterator_(base_iterator)
    {}

    TBaseIterator base_iterator_ = TBaseIterator();
};

// --- TreeChildren

template <typename TChildren>
class TreeChildrenWrapper {
private:
    using Children = TChildren;

public:
    using Tree = typename Children::value_type;

    using Data = typename Tree::Data;
    static constexpr bool leaf_data = Tree::leaf_data;
    static constexpr bool optional_data = Tree::optional_data;

    using Node = TreeNode<Data, leaf_data, optional_data>;

private:
    using Init = detail::TreeInit<Data, leaf_data, optional_data>;

public:
    using value_type = Node;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = TreeNodeIterator<typename Children::iterator>;
    using const_iterator = TreeNodeIterator<typename Children::const_iterator>;
    using reverse_iterator = TreeNodeIterator<typename Children::reverse_iterator>;
    using const_reverse_iterator = TreeNodeIterator<typename Children::const_reverse_iterator>;

    constexpr reference at(size_type pos) { return *children_.at(pos); }
    constexpr const_reference at(size_type pos) const { return *children_.at(pos); }

    constexpr reference operator[](size_type pos) { return *children_[pos]; }
    constexpr const_reference operator[](size_type pos) const { return *children_[pos]; }

    constexpr reference front() { return *children_.front(); }
    constexpr const_reference front() const { return *children_.front(); }

    constexpr reference back() { return *children_.back(); }
    constexpr const_reference back() const { return *children_.back(); }

    constexpr iterator begin() { return children_.begin(); }
    constexpr const_iterator begin() const { return children_.begin(); }
    constexpr const_iterator cbegin() const { return children_.cbegin(); }

    constexpr iterator end() { return children_.end(); }
    constexpr const_iterator end() const { return children_.end(); }
    constexpr const_iterator cend() const { return children_.cend(); }

    constexpr reverse_iterator rbegin() { return children_.rbegin(); }
    constexpr const_reverse_iterator rbegin() const { return children_.rbegin(); }
    constexpr const_reverse_iterator crbegin() const { return children_.crbegin(); }

    constexpr reverse_iterator rend() { return children_.rend(); }
    constexpr const_reverse_iterator rend() const { return children_.rend(); }
    constexpr const_reverse_iterator crend() const { return children_.crend(); }

    constexpr bool empty() const { return children_.empty(); }
    constexpr size_type size() const { return children_.size(); }
    constexpr size_type max_size() const { return children_.max_size(); }
    constexpr void reserve(size_type new_cap) { children_.reserve(new_cap); }
    constexpr size_type capcity() const { return children_.capacity(); }
    constexpr void shrink_to_fit() { children_.shrink_to_fit(); }

    constexpr void clear() { children_.clear(); }
    // insert
    // emplace
    // erase

    constexpr void push_back(const Init& init) { emplace_back(init); }
    constexpr void push_back(Init&& init) { emplace_back(std::move(init)); }

    constexpr reference emplace_back(const Init& init = {})
    {
        auto& tree = children_.emplace_back(init);
        tree->parent_ = this;
        return *tree;
    }

    constexpr reference emplace_back(Init&& init)
    {
        auto& tree = children_.emplace_back(std::move(init));
        tree->parent_ = this;
        return *tree;
    }

    // pop_back
    // resize
    // swap

private:
    template <typename, bool, bool>
    friend class dang::utils::TreeNode;

    TreeChildrenWrapper(Children&& children)
        : children_(std::forward<Children>(children))
    {}

    Children children_;
};

} // namespace detail

// --- Tree

/// @brief An owned pointer to a TreeNode.
/// @remark Unlike std::unique_ptr, it does not support interior mutability.
/// @tparam TData The data for each node/leaf.
/// @tparam v_leaf_data Whether only leaf nodes can data.
/// @tparam v_optional_data Whether data is optional.
template <typename TData, bool v_leaf_data, bool v_optional_data>
class Tree {
public:
    using Data = TData;
    static constexpr bool leaf_data = v_leaf_data;
    static constexpr bool optional_data = v_optional_data;

    using Node = TreeNode<Data, leaf_data, optional_data>;

private:
    using Init = detail::TreeInit<Data, leaf_data, optional_data>;

public:
    constexpr Tree() = default;

    constexpr Tree(const Init& init)
        : node_(std::make_unique<Node>(init))
    {}

    constexpr Tree(Init&& init)
        : node_(std::make_unique<Node>(std::move(init)))
    {}

    constexpr Tree(const Node& node)
        : node_(std::make_unique<Node>(node))
    {}

    constexpr Tree(Node&& node)
        : node_(std::make_unique<Node>(std::move(node)))
    {}

    constexpr Tree(const Tree& other) requires std::copy_constructible<Node>
        : node_(std::make_unique<Node>(*other.node_))
    {}

    constexpr Tree(Tree&&) = default;

    constexpr Tree& operator=(const Tree&) requires std::copy_constructible<Node>
    {
        node_ = std::make_unique<Node>(*node_);
    }

    constexpr Tree& operator=(Tree&&) = default;

    explicit operator bool() const { return node_ != nullptr; }

    constexpr Node& operator*() { return *node_; }
    constexpr const Node& operator*() const { return *node_; }
    constexpr Node* operator->() { return &**this; }
    constexpr const Node* operator->() const { return &**this; }

    // Only compare pointers!
    // operator== and operator!=

private:
    std::unique_ptr<Node> node_;
};

/// @brief A tree node with data and/or children.
/// @tparam TData The data for each node/leaf.
/// @tparam v_leaf_data Whether only leaf nodes can data.
/// @tparam v_optional_data Whether data is optional.
template <typename TData, bool v_leaf_data, bool v_optional_data>
class TreeNode {
public:
    using Data = TData;
    static constexpr bool leaf_data = v_leaf_data;
    static constexpr bool optional_data = v_optional_data;

    using Tree = Tree<Data, leaf_data, optional_data>;

private:
    using Children = detail::TreeChildren<Data, leaf_data, optional_data>;
    using Init = detail::TreeInit<Data, leaf_data, optional_data>;
    using Storage = detail::tree_storage_t<Data, leaf_data, optional_data>;

public:
    constexpr TreeNode() = default;

    constexpr TreeNode(const Init& init)
        : storage_(init)
    {}

    constexpr TreeNode(Init&& init)
        : storage_(std::move(init))
    {}

    TreeNode(const TreeNode&) = delete;
    TreeNode(TreeNode&&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
    TreeNode& operator=(TreeNode&&) = delete;

    // --- Data Access

    // Checked

    constexpr bool hasData() const
    {
        if constexpr (leaf_data)
            return std::holds_alternative<Data>(storage_);
        else {
            if constexpr (optional_data)
                return storage_.data.has_value();
            else
                return true;
        }
    }

    constexpr explicit operator bool() const { return hasData(); }

    constexpr Data* dataOrNull() & { return dataOrNullHelper(*this); }
    constexpr const Data* dataOrNull() const& { return dataOrNullHelper(*this); }

    constexpr void setData(const Data& data)
    {
        if constexpr (leaf_data)
            storage_ = data;
        else
            storage_.data = data;
    }

    constexpr void setData(Data&& data)
    {
        if constexpr (leaf_data)
            storage_ = std::move(data);
        else
            storage_.data = std::move(data);
    }

    constexpr void resetData() requires optional_data
    {
        if constexpr (leaf_data)
            storage_ = std::monostate();
        else
            storage_ = std::nullopt;
    }

    // Unchecked (assumes data)

    constexpr Data& data() & { return dataHelper(*this); }
    constexpr const Data& data() const& { return dataHelper(*this); }
    constexpr Data data() && { return std::move(dataHelper(*this)); }

    constexpr Data& operator*() & { return data(); }
    constexpr const Data& operator*() const& { return data(); }
    constexpr Data operator*() && { return std::move(data()); }

    constexpr Data* operator->() & { return &data(); }
    constexpr const Data* operator->() const& { return &data(); }

    // --- Child Access

    constexpr void supportChildren()
    {
        if constexpr (leaf_data)
            storage_ = children_type();
    }

    constexpr bool supportsChildren() const
    {
        if constexpr (leaf_data)
            return std::holds_alternative<Children>(storage_);
        else
            return true;
    }

    detail::TreeChildrenWrapper<Children&> children() & { return childrenHelper(*this); }
    detail::TreeChildrenWrapper<const Children&> children() const& { return childrenHelper(*this); }
    detail::TreeChildrenWrapper<Children> children() && { return std::move(childrenHelper(*this)); }

    // --- Parent Access

    constexpr TreeNode* parent() { return parent_; }
    constexpr const TreeNode* parent() const { return parent_; }

    constexpr bool isRoot() const { return parent_ == nullptr; }
    constexpr bool isChild() const { return parent_ != nullptr; }

    constexpr TreeNode& root() { return isRoot() ? *this : parent_->root(); }
    constexpr const TreeNode& root() const { return isRoot() ? *this : parent_->root(); }

    // --- Comparison

    // TODO: Default: Compare data first, then children lexicographically by value
    //       Since this is not a binary tree, it doesn't make sense to compare e.g. left/data/right.

private:
    template <typename TSelf>
    static constexpr auto* dataOrNullHelper(TSelf& self)
    {
        if constexpr (leaf_data)
            return std::get_if<Data>(&self.storage_);
        else {
            if constexpr (optional_data)
                return self.storage_.data ? &*self.storage_.data : nullptr;
            else
                return &self.storage_.data;
        }
    }

    template <typename TSelf>
    static constexpr auto& dataHelper(TSelf& self)
    {
        if constexpr (leaf_data)
            return *std::get_if<Data>(&self.storage_);
        else {
            if constexpr (optional_data)
                return *self.storage_.data;
            else
                return self.storage_.data;
        }
    }

    template <typename TSelf>
    static constexpr auto& childrenHelper(TSelf& self)
    {
        if constexpr (leaf_data)
            return *std::get_if<Children>(&self.storage_);
        else
            return self.storage_.children;
    }

    Storage storage_;
    TreeNode* parent_ = nullptr;
};

} // namespace dang::utils
