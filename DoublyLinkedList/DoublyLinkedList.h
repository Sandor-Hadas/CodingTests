#include <memory>
#include <string>

class DoublyLinkedList {
private:
    struct DoublyLinkedListData {
        std::shared_ptr<DoublyLinkedListData> pPrev, pNext;
        void *pData;
    };
    std::shared_ptr<DoublyLinkedListData> pFirst = 0;
    std::shared_ptr<DoublyLinkedListData> pLast = 0;

public:
	DoublyLinkedList();
	
	void push_back(void* ptr);
	void push_front(void* ptr);
    void push_after(void* ptr, std::shared_ptr<DoublyLinkedListData> after);
    void push_before(void* ptr, std::shared_ptr<DoublyLinkedListData> before);
    size_t push_after(void* ptr, void* pAfter, size_t rmax = 1);
    size_t push_before(void* ptr, void* pBefore, size_t rmax = 1);

	void clear();
	size_t removeFirst(void* ptr, size_t rmax = 1);
	size_t removeLast(void* ptr, size_t rmax = 1);
	size_t removeAll(void* ptr);
	void remove(std::shared_ptr<DoublyLinkedListData> ptr);

	// Debug/test methods
	void debugPrintAllElementsForward() const;
	void debugPrintAllElementsReverse() const;
	std::string createTestString() const;
	std::string createTestStringReverse() const;
	size_t getDataCount() const;
};

