#include "DoublyLinkedList.h"

#include <iostream>
#include <iomanip>

DoublyLinkedList::DoublyLinkedList() {
    pFirst = 0;
    pLast = 0;
}

void DoublyLinkedList::push_back(void* ptr) {
    std::shared_ptr<struct DoublyLinkedListData> newData(new DoublyLinkedListData);
    newData->pData = ptr;
    newData->pPrev = pLast;
    newData->pNext = 0;
    
    if (pLast != 0) {
        pLast->pNext = newData;
    }
    pLast = newData;
	if (pFirst == 0) {
		pFirst = newData;
	}
}

void DoublyLinkedList::push_front(void* ptr) {
    std::shared_ptr<struct DoublyLinkedListData> newData(new DoublyLinkedListData);
    newData->pData = ptr;
    newData->pPrev = 0;
    newData->pNext = pFirst;
    
    pFirst = newData;
	if (pLast == 0) {
		pLast = newData;
	} else {
		newData->pNext->pPrev = newData;
	}
}

void DoublyLinkedList::push_after(void* ptr, std::shared_ptr<DoublyLinkedListData> after) {
    std::shared_ptr<DoublyLinkedListData> newData(new DoublyLinkedListData);
    newData->pData = ptr;
    newData->pNext = after->pNext;
    newData->pPrev = after;
    if (after->pNext == nullptr) {
        pLast = newData;
    } else {
        after->pNext->pPrev = newData;
    }
    after->pNext = newData;
}

void DoublyLinkedList::push_before(void* ptr, std::shared_ptr<DoublyLinkedListData> before) {
    std::shared_ptr<DoublyLinkedListData> newData(new DoublyLinkedListData);
    newData->pData = ptr;
    newData->pPrev = before->pPrev;
    newData->pNext = before;
    if (newData->pPrev == nullptr) {
        pFirst = newData;
    } else {
        newData->pPrev->pNext = newData;
    }
    before->pPrev = newData;
}

size_t DoublyLinkedList::push_after(void* ptr, void* pAfter, size_t rmax) {
	if (rmax < 1) {
		return 0;
	}

	size_t added = 0;
	std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
    	if (iter->pData == pAfter) {
    		push_after(ptr, iter);
    		added++;
    		if (added == rmax) {
    			break;
    		}
    	}
    	iter = iter->pNext;
    } while (iter != 0);

    return added;
}

size_t DoublyLinkedList::push_before(void* ptr, void* pBefore, size_t rmax) {
	if (rmax < 1) {
		return 0;
	}

	size_t added = 0;
	std::shared_ptr<DoublyLinkedListData> iter = pLast;
    do {
    	if (iter->pData == pBefore) {
    		push_before(ptr, iter);
    		added++;
    		if (added == rmax) {
    			break;
    		}
    	}
    	iter = iter->pPrev;
    } while (iter != 0);

    return added;
}

void DoublyLinkedList::clear() {
	if (pFirst == 0) {
		return;
	}

	std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
        iter->pPrev = 0;
        iter->pData = 0;
        if (iter->pNext != 0) {
        	iter = iter->pNext;
        }
    } while (iter == 0);

    pFirst = 0;
    pLast = 0;

}

void DoublyLinkedList::remove(std::shared_ptr<DoublyLinkedListData> ptr) {
    if (ptr->pPrev == 0) {
    	pFirst = ptr->pNext;
    } else {
    	ptr->pPrev->pNext = ptr->pNext;
    }

    if (ptr->pNext == 0) {
    	pLast = ptr->pPrev;
    } else {
    	ptr->pNext->pPrev = ptr->pPrev;
    }

    ptr->pPrev = 0;
    ptr->pData = 0;
    ptr->pNext = 0;
}

size_t DoublyLinkedList::removeFirst(void* ptr, size_t rmax) {
	if (rmax < 1) {
		return 0;
	}

	size_t removed = 0;
	std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
    	if (iter->pData == ptr) {
    		remove(iter);
    		removed++;
    		if (removed == rmax) {
    			break;
    		}
    	}
    	iter = iter->pNext;
    } while (iter != 0);

    return removed;
}

size_t DoublyLinkedList::removeLast(void* ptr, size_t rmax) {
	if (rmax < 1) {
		return 0;
	}

	size_t removed = 0;
	std::shared_ptr<DoublyLinkedListData> iter = pLast;
    do {
    	if (iter->pData == ptr) {
    		remove(iter);
    		removed++;
    		if (removed == rmax) {
    			break;
    		}
    	}
    	iter = iter->pPrev;
    } while (iter != 0);

    return removed;
}

size_t DoublyLinkedList::removeAll(void* ptr) {
	size_t removed = 0;

	std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
    	if (iter->pData == ptr) {
    		remove(iter);
    		removed++;
    	}
        iter = iter->pNext;
    } while (iter != 0);

    return removed;
}

void DoublyLinkedList::debugPrintAllElementsForward() const {
    std::cout << "debugPrintAllElementsForward:" << std::endl;
    if (pFirst == 0) {
        std::cout << "The list is empty" << std::endl << std::endl;
        return;
    }

    std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
        std::cout << std::setfill('0') << std::setw(16) << std::right << std::hex << iter->pPrev << " "
        << std::setfill('0') << std::setw(16) << std::right << std::hex << iter << " "
        << std::setfill('0') << std::setw(16) << std::right << std::hex << iter->pNext << "   "
        << std::setfill('0') << std::setw(16) << std::right << std::hex  << iter->pData << std::endl;
        iter = iter->pNext;
    } while (iter != 0);
}

void DoublyLinkedList::debugPrintAllElementsReverse() const {
    std::cout << "debugPrintAllElementsReverse:" << std::endl;
    if (pFirst == 0) {
        std::cout << "The list is empty" << std::endl << std::endl;
        return;
    }

    std::shared_ptr<DoublyLinkedListData> iter = pLast;
    do {
        std::cout << std::setfill('0') << std::setw(16) << std::right << std::hex << iter->pPrev << " "
        << std::setfill('0') << std::setw(16) << std::right << std::hex << iter << " "
        << std::setfill('0') << std::setw(16) << std::right << std::hex << iter->pNext << "   "
        << std::setfill('0') << std::setw(16) << std::right << std::hex  << iter->pData << std::endl;
        iter = iter->pPrev;
    } while (iter != 0);
}

std::string DoublyLinkedList::createTestString() const {
	if (pFirst == 0) {
		return "";
	}

	std::string out;

	std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
    	out += (const char *)iter->pData;
    	iter = iter->pNext;
        if (iter != 0) {
        	out += ",";
        }
    } while (iter != 0);

    return out;
}

std::string DoublyLinkedList::createTestStringReverse() const {
	if (pFirst == 0) {
		return "";
	}

	std::string out;

	std::shared_ptr<DoublyLinkedListData> iter = pLast;
    do {
    	out += (const char *)iter->pData;
    	iter = iter->pPrev;
        if (iter != 0) {
        	out += ",";
        }
    } while (iter != 0);

    return out;
}

size_t DoublyLinkedList::getDataCount() const {
	if (pFirst == 0) {
        return 0;
    }

    size_t counter = 0;
    std::shared_ptr<DoublyLinkedListData> iter = pFirst;
    do {
        iter = iter->pNext;
        counter++;
    } while (iter != 0);

    return counter;
}
