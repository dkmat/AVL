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

std::string AVLTree::JSON()const  {
	nlohmann::json result;
	std::queue< std::shared_ptr<BSTNode> > nodes;
	if (root_ != nullptr) {
		result["root"] = root_->key_;
		result["height"] = root_->height_;
		nodes.push(root_);
		while (!nodes.empty()) {
			auto v = nodes.front();
			nodes.pop();
			std::string key = std::to_string(v->key_);
			result[key]["balance factor"] = v->bf_;
			result[key]["height"] = v->height_;
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

void AVLTree::readFile(const std::string &fileName){
	std::ifstream file; // creating a file variable to open the file
    file.open(fileName); // opening the file
    nlohmann::json fileInfo; // creating a JSON object
    if(file.is_open()){ // checking if file is open
        file>>fileInfo; // entering file data into the JSON object
    }
    file.close(); // closing the file
    parseFile(fileInfo);
}

void AVLTree::parseFile(nlohmann::json &fileInfo){
	int num = fileInfo["metadata"]["numOps"]; //getting the correct number of operations
	std::string temp; // string to access each operation
	for(int i =1; i<=num; i++){
		if(num<=99){
			if(i<10){
				temp = "0"+std::to_string(i);
			}
			else{
				temp = std::to_string(i);
			}
		}
		else if(num<=1000){
			if(i<10){
				temp = "000"+std::to_string(i);
			}
			else if(i<100){
				temp = "00"+std::to_string(i);
			}
			else if(i<1000){
				temp = "0"+std::to_string(i);
			}
			else{
				temp = std::to_string(i);
			}
		}
		if(fileInfo[temp]["operation"]=="Insert"){ //checking the operation is insert
			Insert(fileInfo[temp]["key"]);
		}
	}
	std::cout<< JSON(); //printing out the output to the screen
}

void AVLTree::Insert(int key) { // function to insert to the AVL tree
	//std::cout<<"insert\n";
	if (root_ == nullptr) {
		root_ = std::make_shared<BSTNode>(key);
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
	} else if(key>lastNode->key_) {
		lastNode->right_ = std::make_shared<BSTNode>(key, lastNode);
	}else{
		size_--;
	}
	size_++;
	height(root_); //calling function from root to make sure everything is updated
}

int AVLTree::height(std::shared_ptr<BSTNode> node){
	if(node == nullptr){ // empty tree
		return -1;
	}
	if(node->IsLeaf()){ // leaf of the tree
		node->height_ = 0;
		node->bf_ = 0;
		return 0;
	}
	int l_height = height(node->left_); //recursive call to check left node height
	int r_height = height(node->right_); //recursive call to check right node height
	node->height_ = std::max(l_height,r_height)+1; //getting correct height for current node
	if(l_height == r_height){ //making sure we have balance factor 0 for equal heights
		node->bf_ = 0;
	}
	/*else if(std::max(l_height,r_height) == l_height) {
		node->bf_ = r_height - l_height;
	}*/ else {
		node->bf_ = r_height - l_height; //balance factor is always right - left if unequal heights
	}
	//std::cout<<node->key_<<" "<<node->bf_<<std::endl;
	if(abs(node->bf_) > 1) { //checking if node is imbalanced
		balance(node); //balancing the tree from the current node
	}
	return node->height_; // returning correct height
}

void AVLTree::balance(std::shared_ptr<BSTNode> node){
	//std::cout<<"imbalance\n";
	if(node->bf_<0){
		if(node->left_->bf_<=0){
			//std::cout<<"R\n";
			Rrotation(node);//LL problem RR rotation
		}else {
			//std::cout<<"RL\n";
			RLrotation(node);//LR problem RL rotation
		}
	}else if(node->bf_>0){
		if(node->right_->bf_>=0){
			//std::cout<<"L\n";
			Lrotation(node);//RR problem LL rotation
		}else{
			//std::cout<<"LR\n";
			LRrotation(node);//RL problem LR rotation
		}
	}
}

void AVLTree::Rrotation(std::shared_ptr<BSTNode> node){ //right rotation
	std::shared_ptr<BSTNode> x = node->left_; // setting left side to not lose it
	if(node->left_->HasLeftChild()){ //checking if left has a child so we don't dereference null pointer
		node->left_->left_->parent_ = x;
	}
	std::shared_ptr<BSTNode> T2 = x->right_;
	if(x->HasRightChild()){ //checking if right has child so we don't dereference null pointer
		T2->parent_ = node;
	}
	x->right_ = node; //doing the correct right rotation
	node->left_ = T2; //setting the correct child
	if(node->parent_.lock()==nullptr){//checking if node is the root
		x->parent_.reset();
		root_ = x; //setting new root
	}
	else{
		x->parent_ = node->parent_; //new parent because node is not root
		if(node->parent_.lock()->right_==node){//checking for correct child of parent
			node->parent_.lock()->right_ = x;
		}
		else{
			node->parent_.lock()->left_ = x;
		}
	}
	node->parent_ = x;
	//std::cout<<"right\n";
	height(x); //calling height to rebalance after rotation
}

void AVLTree::Lrotation(std::shared_ptr<BSTNode> node){ //same as right rotation but everything is opposite
	std::shared_ptr<BSTNode> y = node->right_;
	if(node->right_->HasRightChild()){
		node->right_->right_->parent_ = y;
	}
	std::shared_ptr<BSTNode> T2 = y->left_;
	if(y->HasLeftChild()){
		T2->parent_=node;
	}
	y->left_ = node;
	node->right_ = T2;
	if(node->parent_.lock()==nullptr){
		y->parent_.reset();
		root_ = y;
	}
	else{
		y->parent_ = node->parent_;
		if(node->parent_.lock()->right_==node){
			node->parent_.lock()->right_ = y;
		}
		else{
			node->parent_.lock()->left_ = y;
		}
	}
	node->parent_ = y;
	//std::cout<<"left\n";
	height(y);
}

void AVLTree::RLrotation(std::shared_ptr<BSTNode> node){
	Lrotation(node->left_);
	Rrotation(node);
}

void AVLTree::LRrotation(std::shared_ptr<BSTNode> node){
	Rrotation(node->right_);
	Lrotation(node);
}