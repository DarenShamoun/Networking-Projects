#include <queue>
#include <mutex>
#include <condition_variable>
#include "ClientSocket.hpp"

/**
 * Class representing a buffer with a fixed capacity.
 *
 * Note that in C++, the header (i.e. hpp) file contains a declaration of the
 * class while the implementation of the constructors, destructors, and methods,
 * and given in an implementation (i.e. cpp) file.
 */
class BoundedBuffer {
  // begin section containing publicly accessible parts of the class
  public:
	  // public constructor
	  BoundedBuffer(int max_size);
	  
	  // public member functions (a.k.a. methods)
	  ClientSocket getItem();
	  void putItem(ClientSocket new_item);

  // begin section containing private (i.e. hidden) parts of the class
  private:
	  // private member variables (i.e. fields)
	  std::size_t capacity;
	  std::queue<ClientSocket> buffer;
	  std::mutex *shared_mutex;
	  std::condition_variable *data_available;
	  std::condition_variable *space_available;

	  // This class doesn't have any, but we could also have private
	  // constructors and/or member functions here.
};
