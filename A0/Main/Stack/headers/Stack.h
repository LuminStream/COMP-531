#ifndef STACK_H
#define STACK_H

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:

	Data holdMe;
	Node *next;
	
public:

	// Constructor
	Node(Data val) {
		holdMe = val;
		next = NULL;
	}
	
	// Return the value of current node
	Data getVal() {
		return holdMe;
	}
	
	// Return the next node
	Node* getNext() {
		return next;
	}
	
	// Set the next node
	void setNext(Node* node) {
		next = node;
	}

};

// a simple LIFO stack
template <class Data> 
class Stack {

	Node <Data> *head;

public:

	// destroys the stack
	~Stack () {
		while(head != NULL){
			delete(head);
			head = head->getNext();
		}
	}

	// creates an empty stack
	Stack () {
		head = NULL;
	}

	// adds pushMe to the top of the stack
	void push (Data val) {
		Node<Data> *newNode = new Node<Data>(val);
		// If there is no head then set as head
		if(head == NULL){
			head = newNode;
		}
		// Else set newNode as new head
		else{
			newNode->setNext(head);
			head = newNode;
		}
	}

	// return true if there are not any items in the stack
	bool isEmpty () {
		if(head == NULL){
			return true;
		}
		else{
			return false;
		}
	}

	// pops the item on the top of the stack off, returning it...
	// if the stack is empty, the behavior is undefined
	Data pop () {
		// return the value of head and set new head as the next node
		Node<Data> *topNode = head;
		head = head->getNext();
		Data val = topNode->getVal();
		delete(topNode);
		return val;
	}
};

#endif
