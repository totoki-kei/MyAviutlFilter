#pragma once

#include <cassert>

class LinkList {

public:
	struct Node {
		Node* prev;
		Node* next;

		Node() : prev(nullptr), next(nullptr) {
			__noop();
		}

		Node(const Node&) = delete;
		Node(Node&&) = delete;

		Node& operator =(const Node&) = delete;
		Node& operator =(Node&&) = delete;



		virtual ~Node() {
			Purge();
		}



		void InsertBefore(Node* node) {
			_ASSERT(this->next);
			_ASSERT(this->prev);
			_ASSERT(this->next->prev == this);
			_ASSERT(this->prev->next == this);

			// 操作のアトミックせいの保証とかは今はしない
			// 適当に外側でクリティカルセクションでも使って欲しい
			if (node == this) return;
			node->next = this;
			node->prev = this->prev;
			this->prev->next = node;
			this->prev = node;

			_ASSERT(this->next->prev == this);
			_ASSERT(this->prev->next == this);
			_ASSERT(node->next->prev == node);
			_ASSERT(node->prev->next == node);
		}

		void InsertAfter(Node* node) {
			_ASSERT(this->next->prev == this);
			_ASSERT(this->prev->next == this);

			// 操作のアトミックせいの保証とかは今はしない
			// 適当に外側でクリティカルセクションでも使って欲しい
			if (node == this) return;
			node->next = this->next;
			node->prev = this;
			this->next->prev = node;
			this->next = node;

			_ASSERT(this->next->prev == this);
			_ASSERT(this->prev->next == this);
			_ASSERT(node->next->prev == node);
			_ASSERT(node->prev->next == node);
		}

		void Purge() {
			//if (prev && prev != this && prev->next == this) {
			//	prev->next = next;
			//}
			//if (next && next != this && next->prev == this) {
			//	next->prev = prev;
			//}
			if (prev) {
				prev->next = next;
			}
			if (next) {
				next->prev = prev;
			}
			prev = next = nullptr;

		}
	};

private:
	Node root_;

public:
	LinkList()
	{
		root_.prev = root_.next = &root_;
	}

	Node* Head() {
		return root_.next != &root_ ? root_.next : nullptr;
	}

	Node* Tail() {
		return root_.prev != &root_ ? root_.prev : nullptr;
	}

	Node* Root() {
		return &root_;
	}

	void InsertBefore(Node* node, Node* new_node) { node->InsertBefore(new_node); }
	void InsertAfter(Node* node, Node* new_node) { node->InsertAfter(new_node); }

	void InsertHead(Node* node) { InsertAfter(Root(), node); }
	void InsertTail(Node* node) { InsertBefore(Root(), node); }

	void Remove(Node* node) { node->Purge(); }

	template <typename Fn>
	void ForEach(Fn fn) {
		for (Node* node = Head(); node && node != Root(); node = node->next) {
			if (fn(node)) break;
		}
	}
};
