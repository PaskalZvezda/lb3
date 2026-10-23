#include <algorithm>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>

template<typename T, typename Step>
class Iterator {
public:
    explicit Iterator(T *ptr) : ptr_(ptr) {}
    auto &operator*() const { return Step::value(ptr_); }
    auto &get() const { return Step::value(ptr_); }
    Iterator &operator++() { ptr_ = Step::next(ptr_); return *this; }
    bool operator!=(const Iterator &other) const { return ptr_ != other.ptr_; }

private:
    T *ptr_;
};

template<typename T>
class SeqContainer {
    struct Step {
        static T &value(T *p) { return *p; }
        static T *next(T *p) { return p + 1; }
    };

public:
    SeqContainer() = default;
    SeqContainer(SeqContainer &&other) noexcept { swap(other); }
    SeqContainer &operator=(SeqContainer &&other) noexcept { swap(other); return *this; }
    ~SeqContainer() { delete[] data_; }

    void push_back(const T &value) { T copy(value); insert(size_, std::move(copy)); }
    void push_back(T &&value) { insert(size_, std::move(value)); }

    void insert(std::size_t pos, T &&value) {
        if (size_ == capacity_) {
            capacity_ = capacity_ + capacity_ / 2 + 1;
            T *data = new T[capacity_];
            std::move(data_, data_ + size_, data);
            delete[] data_;
            data_ = data;
        }
        std::move_backward(data_ + pos, data_ + size_, data_ + size_ + 1);
        data_[pos] = std::move(value);
        ++size_;
    }

    void erase(std::size_t pos) {
        std::move(data_ + pos + 1, data_ + size_, data_ + pos);
        --size_;
    }

    std::size_t size() const { return size_; }
    T &operator[](std::size_t index) { return data_[index]; }
    Iterator<T, Step> begin() { return Iterator<T, Step>(data_); }
    Iterator<T, Step> end() { return Iterator<T, Step>(data_ + size_); }

private:
    void swap(SeqContainer &other) {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    T *data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

template<typename T>
struct DoublyNode {
    T value;
    DoublyNode *next;
    DoublyNode *prev;
};

template<typename T>
struct SinglyNode {
    T value;
    SinglyNode *next;
};

template<typename T, template<typename> class NodeT>
class List {
    using Node = NodeT<T>;
    static constexpr bool doubly = std::is_same_v<Node, DoublyNode<T>>;
    struct Step {
        static T &value(Node *n) { return n->value; }
        static Node *next(Node *n) { return n->next; }
    };

public:
    List() = default;
    List(List &&other) noexcept { swap(other); }
    List &operator=(List &&other) noexcept { swap(other); return *this; }
    ~List() { while (size_) erase(0); }

    void push_back(const T &value) { T copy(value); insert(size_, std::move(copy)); }
    void push_back(T &&value) { insert(size_, std::move(value)); }

    void insert(std::size_t pos, T &&value) {
        Node **link = link_at(pos);
        if constexpr (doubly) {
            *link = new Node{std::move(value), *link, pos ? *link_at(pos - 1) : nullptr};
            if ((*link)->next)
                (*link)->next->prev = *link;
        } else {
            *link = new Node{std::move(value), *link};
        }
        ++size_;
    }

    void erase(std::size_t pos) {
        Node **link = link_at(pos);
        Node *node = *link;
        *link = node->next;
        if constexpr (doubly)
            if (node->next)
                node->next->prev = node->prev;
        delete node;
        --size_;
    }

    std::size_t size() const { return size_; }
    T &operator[](std::size_t index) { return (*link_at(index))->value; }
    Iterator<Node, Step> begin() { return Iterator<Node, Step>(head_); }
    Iterator<Node, Step> end() { return Iterator<Node, Step>(nullptr); }

private:
    Node **link_at(std::size_t pos) {
        Node **link = &head_;
        for (std::size_t i = 0; i < pos; ++i)
            link = &(*link)->next;
        return link;
    }

    void swap(List &other) {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
    }

    Node *head_ = nullptr;
    std::size_t size_ = 0;
};

template<typename T>
using ListContainer = List<T, DoublyNode>;
template<typename T>
using ForwardListContainer = List<T, SinglyNode>;

template<typename Container>
void print(Container &container) {
    const char *sep = "";
    for (auto &value : container) {
        std::cout << sep << value;
        sep = ", ";
    }
    std::cout << std::endl;
}

template<typename Container>
void demo(const char *name) {
    std::cout << name << std::endl;
    Container container;
    for (int i = 0; i < 10; ++i)
        container.push_back(i);
    print(container);
    std::cout << container.size() << std::endl;

    container.erase(2);
    container.erase(3);
    container.erase(4);
    print(container);

    container.insert(0, 10);
    print(container);
    container.insert(container.size() / 2, 20);
    print(container);
    container.push_back(30);
    print(container);

    Container moved = std::move(container);
    std::cout << "move: " << moved.size() << " " << container.size() << std::endl;
    std::cout << "[4] = " << moved[4] << std::endl << std::endl;
}

int main() {
    demo<SeqContainer<int>>("SeqContainer");
    demo<ListContainer<int>>("ListContainer");
    demo<ForwardListContainer<int>>("ForwardListContainer");
    return 0;
}
