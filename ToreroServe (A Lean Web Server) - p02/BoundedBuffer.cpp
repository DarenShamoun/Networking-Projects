/**
 * Implementation of the BoundedBuffer class.
 * See the associated header file (BoundedBuffer.hpp) for the declaration of
 * this class.
 */
#include <cstdio>

#include "BoundedBuffer.hpp"
#include <mutex>

/**
 * Constructor that sets capacity to the given value. The buffer itself is
 * initialized to en empty queue.
 *
 * @param max_size The desired capacity for the buffer.
 */
BoundedBuffer::BoundedBuffer(int max_size) {
	capacity = std::size_t(max_size);
	std::mutex *shared_mutex = new std::mutex();
	std::condition_variable *data_available = new std::condition_variable();
	std::condition_variable *space_available = new std::condition_variable();

	this->shared_mutex = shared_mutex;
	this->data_available = data_available;
	this->space_available = space_available;

	// buffer field implicitly has its default (no-arg) constructor called.
	// This means we have a new buffer with no items in it.
}

/**
 * Gets the first item from the buffer then removes it.
 *
 * @return The value taken from the front of the buffer.
 */
ClientSocket BoundedBuffer::getItem() {
	std::unique_lock<std::mutex> lock(*shared_mutex);

	while (buffer.size() == 0) {
		data_available->wait(lock);
	}

	ClientSocket item = this->buffer.front(); // "this" refers to the calling object...
	buffer.pop(); // ... but like Java it is optional (no this in front of buffer on this line)
	return item;
}

/**
 * Adds a new item to the back of the buffer.
 *
 * @param new_item The item to put in the buffer.
 */
void BoundedBuffer::putItem(ClientSocket new_item) {
	std::unique_lock<std::mutex> lock(*shared_mutex);
	
	while (buffer.size() == capacity) {
		space_available->wait(lock);
	}
	
	buffer.push(new_item);

	data_available->notify_one();
}
