/**
 * @file
 */

#include <stdint.h>

struct SDL_Semaphore;

namespace core {

enum class SemaphoreWaitState { Success, WouldBlock, Error };

/**
 * @note Each @c waitAndDecrease() operation will atomically decrement the value and potentially block if the semaphore
 * value is 0. Each @c increase() operation will atomically increment the value and wake potentially waiting threads.
 */
class Semaphore {
private:
	SDL_Semaphore *_semaphore;

public:
	/**
	 * @brief Construct a new Semaphore object
	 *
	 * @param initialValue The initial value defines how many times the code can pass through the semaphore before the
	 * lock is held.
	 */
	Semaphore(uint32_t initialValue = 1);
	~Semaphore();

	/**
	 * @brief This function suspends the calling thread until the semaphore has a positive count. It then atomically
	 * decreases the semaphore count.
	 * @return false on error
	 */
	bool waitAndDecrease();
	/**
	 * @brief Atomically increases the semaphore's count (not blocking).
	 * @return false on error
	 */
	bool increase();
	SemaphoreWaitState tryWait();
	bool waitTimeout(uint32_t timeout);
	/**
	 * @brief Get the current value of the semaphore.
	 */
	uint32_t value();
};

} // namespace core
