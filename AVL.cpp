#include "AVL.h"

#include <cassert>
#include <iostream>
#include <string>
#include <queue>

#include "json.hpp"


BSTNode::BSTNode(int key) :
	key_(key),
	parent_(std::weak_ptr<BSTNode>()),
	left_(nullptr),
	right_(nullptr) {}

BSTNode::BSTNode(int key, std::weak_ptr<BSTNode> parent) :
	key_(key),
	parent_(parent),
	left_(nullptr),
	right_(nullptr) {}

bool BSTNode::IsLeaf() const {
	return left_ == nullptr && right_ == nullptr;
}

bool BSTNode::HasLeftChild() const {
	return left_ != nullptr;
}

bool BSTNode::HasRightChild() const {
	return right_ != nullptr;
}

void BSTNode::DeleteChild(std::shared_ptr<BSTNode> v) {
	if (left_ == v) {
		left_ = nullptr;
	} else if (right_ == v) {
		right_ = nullptr;
	} else {
		std::cerr << "BSTNode::DeleteChild Error: non-child passed as argument\n";
		exit(EXIT_FAILURE);
	}
}

void BSTNode::ReplaceChild(std::shared_ptr<BSTNode> v, std::shared_ptr<BSTNode> u) {
	if (left_ == u || right_ == u) {
		std::cerr << "BSTNode::ReplaceChild Error: child passed as replacement\n";
	}
	if (left_ == v) {
		left_ = u;
		u->parent_ = v->parent_;
	} else if (right_ == v) {
		right_ = u;
		u->parent_ = v->parent_;
	} else {
		std::cerr << "BSTNode::ReplaceChild Error: non-child passed as argument\n";
		exit(EXIT_FAILURE);
	}
}

AVLTree::AVLTree() : root_(nullptr), size_(0) {}

void AVLTree::Insert(int key) {
	if (root_ == nullptr) {
		root_ = std::make_shared<BSTNode>(key);
		root_->bf_=0;
		size_++;
		return;
	}
	std::shared_ptr<BSTNode> currentNode = root_, lastNode = nullptr;
	while (currentNode != nullptr) {
		lastNode = currentNode;
		currentNode = (key < currentNode->key_) ?
			currentNode->left_ : currentNode->right_;
	}
	if (key < lastNode->key_) {
		lastNode->left_ = std::make_shared<BSTNode>(key, lastNode);
		lastNode->bf_ = lastNode->bf_ - 1;
	} else {
		lastNode->right_ = std::make_shared<BSTNode>(key, lastNode);
		lastNode->bf_ = lastNode->bf_ + 1;
	}
	size_++;
}

bool AVLTree::Delete(int key) {
	std::shared_ptr<BSTNode> currentNode = root_;
	while (currentNode != nullptr) {
		if (currentNode->key_ == key) {
			if (currentNode->IsLeaf()) {
				DeleteLeaf(currentNode);
			} else if (currentNode->left_ == nullptr) {
				assert(currentNode->right_ != nullptr);
				std::shared_ptr<BSTNode> parent = currentNode->parent_.lock();
				parent->ReplaceChild(currentNode, currentNode->right_);
				size_--; assert(size_ >= 0);
			} else if (currentNode->right_ == nullptr) {
				assert(currentNode->left_ != nullptr);
				std::shared_ptr<BSTNode> parent = currentNode->parent_.lock();
				parent->ReplaceChild(currentNode, currentNode->left_);
				size_--; assert(size_ >= 0);
			} else {
				currentNode->key_ = DeleteMin(currentNode);
			}
		}
		currentNode = (key < currentNode->key_) ?
			currentNode->left_ : currentNode->right_;
	}
	return false;
}

int AVLTree::DeleteMin() {
	return DeleteMin(root_);
}


void AVLTree::DeleteLeaf(std::shared_ptr<BSTNode> currentNode) {
	std::shared_ptr<BSTNode> parent = currentNode->parent_.lock();
	if (parent == nullptr) {
		// Delete root
		root_ = nullptr;
		size_--; assert(size_ == 0);
	} else {
		if (parent->right_ == currentNode) {
			parent->right_ = nullptr;
		} else if (parent->left_ == currentNode) {
			parent->left_ = nullptr;
		} else {
			std::cerr << "BST::DeleteLeaf Error: inconsistent state\n";
		}
		size_--; assert(size_ >= 0);
	}
}

int AVLTree::DeleteMin(std::shared_ptr<BSTNode> currentNode) {
	std::shared_ptr<BSTNode> lastNode = nullptr;
	while (currentNode != nullptr) {
		lastNode = currentNode;
		currentNode = currentNode->left_;
	}
	int result = lastNode->key_;
	std::shared_ptr<BSTNode> parent = lastNode->parent_.lock();
	if (parent == nullptr) {
		// lastNode is root
		if (lastNode->right_ != nullptr) {
			root_ = lastNode->right_;
			lastNode->right_->parent_.reset();
		} else {
			root_ = nullptr;
		}
	} else {
		// lastNode under the root
		if (lastNode->right_ != nullptr) {
			parent->left_ = lastNode->right_;
			lastNode->right_->parent_ = parent;
		} else {
			parent->left_ = nullptr;
		}
  }
	size_--; assert(size_ >= 0);
	return result;
}

size_t AVLTree::size() const {
	return size_;
}

bool AVLTree::empty() const {
	return size_ == 0;
}

bool AVLTree::Find(int key) const {
	std::shared_ptr<BSTNode> currentNode = root_;
	while (currentNode != nullptr) {
		if (currentNode->key_ == key) {
			return true;
		}
		currentNode = (key < currentNode->key_) ?
			currentNode->left_ : currentNode->right_;
	}
	return false;
}

std::string AVLTree::JSON() const {
	nlohmann::json result;
	std::queue< std::shared_ptr<BSTNode> > nodes;
	if (root_ != nullptr) {
		result["root"] = root_->key_;
		nodes.push(root_);
		while (!nodes.empty()) {
			auto v = nodes.front();
			nodes.pop();
			std::string key = std::to_string(v->key_);
			if (v->left_ != nullptr) {
				result[key]["left"] = v->left_->key_;
				nodes.push(v->left_);
			}
			if (v->right_ != nullptr) {
				result[key]["right"] = v->right_->key_;
				nodes.push(v->right_);
			}
			if (v->parent_.lock() != nullptr) {
				result[key]["parent"] = v->parent_.lock()->key_;
			} else {
				result[key]["root"] = true;
			}
		}
	}
	result["size"] = size_;
	return result.dump(2) + "\n";
}
