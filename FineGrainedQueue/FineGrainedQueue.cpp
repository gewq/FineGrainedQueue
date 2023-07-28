#include "FineGrainedQueue.h"
#include <cassert>
#include <thread>
#include <iostream>

#include "Exceptions/ListIsEmpty_Exception.h"


FineGrainedQueue::FineGrainedQueue(): size_(0), head_(nullptr), tail_(nullptr)
{
}



FineGrainedQueue::FineGrainedQueue(std::initializer_list<int> values):
  FineGrainedQueue()
{
  for (const auto value : values){
    pushBack(value);
  }
}



FineGrainedQueue::~FineGrainedQueue()
{
}



void FineGrainedQueue::pushFront(int value)
{
  //Создать новый элемент
  std::shared_ptr<Node> nodeNew = std::make_shared<Node>(value);

  //Захватить одновременно mutex начала и конца списка
  std::lock(mutexHead_, mutexTail_);

  //Список пуст
  if (!head_){
    head_ = nodeNew;
    tail_ = nodeNew;
    mutexHead_.unlock();
    mutexTail_.unlock();
  }

  //Список не пуст
  else{
    //Добавляем в начало списка - поэтому tail_ не нужен
    mutexTail_.unlock();
    //Новый элемент указывает на первый элемент
    nodeNew->next = head_;
    //Новый элемент становится первым
    head_ = nodeNew;
    mutexHead_.unlock();
  }
  ++size_;
}



void FineGrainedQueue::pushBack(int value)
{
  //Создать новый элемент
  std::shared_ptr<Node> nodeNew = std::make_shared<Node>(value);

  //Захватить одновременно mutex начала и конца списка
  std::lock(mutexHead_, mutexTail_);

  //Список пуст
  if (!head_){
    head_ = nodeNew;
    tail_ = nodeNew;
    mutexHead_.unlock();
    mutexTail_.unlock();
  }

  //Список не пуст
  else{
    //Добавляем в конец списка - поэтому head_ не нужен
    mutexHead_.unlock();
    //Последний элемент указывает на новый элемент
    tail_->next = nodeNew;
    //Новый элемент становится последним
    tail_ = nodeNew;
    mutexTail_.unlock();
  }
  ++size_;
}



void FineGrainedQueue::insertIntoMiddle(int value, size_t pos)
{
  if (pos == 0){
    pushFront(value);
  }
  else if (pos >= size_){
    pushBack(value);
  }
  else{
    std::shared_ptr<Node> nodeNew = std::make_shared<Node>(value);

    //Двигаясь вперёд по списку захватываем mutex элемента и освобождаем
    //mutex предыдущего элемента - ищем элемент, предыдущий элементу
    //с заданной позицией в списке
    size_t currentPos = 0;
    mutexHead_.lock();
    std::shared_ptr<Node> iter = head_;
    std::shared_ptr<Node> iterPrev = nullptr;
    iter->mutex.lock(); //Захватить mutex первого элемента
    mutexHead_.unlock();

    while(iter->next){
      if (currentPos == pos-1){
        break;
      }
      iterPrev = iter;
      iter = iter->next;
      iter->mutex.lock();
      iterPrev->mutex.unlock();
      ++currentPos;
    }
    //iter указывает на pos-1 элемент, его mutex захвачен
    //mutex pos-1-1 освобождён
    nodeNew->next = iter->next;
    iter->next = nodeNew;
    iter->mutex.unlock();
    ++size_;
  }
}



size_t FineGrainedQueue::getSize() const
{
  return size_;
}



int FineGrainedQueue::getValue(size_t pos) const
{
  //Обработка ошибок
  if (isEmpty()){
    throw ListIsEmpty_Exception();
  }
  if (pos > size_-1){
    const std::string errorMessage = "Error: pos (" +
      std::to_string(pos) + ") is out_of_range";
		throw std::out_of_range(errorMessage.c_str());
  }
  //Найти элемент pos
  size_t currentPos = 0;
  mutexHead_.lock_shared();
  std::shared_ptr<Node> iter = head_;
  std::shared_ptr<Node> iterPrev = nullptr;
  iter->mutex.lock_shared(); //Залочить mutex первого элемента
  mutexHead_.unlock_shared();

  while(iter->next){
    if (currentPos == pos){
      break;
    }
    iterPrev = iter;
    iter = iter->next;
    iter->mutex.lock_shared();
    iterPrev->mutex.unlock_shared();
    ++currentPos;
  }
  //iter указывает на pos элемент, его mutex захвачен
  //mutex pos-1 освобождён
  const int resultValue = iter->value;
  iter->mutex.unlock_shared();
  return resultValue;
}



bool FineGrainedQueue::isEmpty() const
{
  if (size_ == 0){
    return true;
  }
  return false;
}


//=============================================================================
static void testCtor();
static void testPushFront();
static void testPushBack();
static void testInsertIntoMiddle();
static void testGetValue();
static void testIsEmpty();


void fine_grained_queue::test()
{
  testCtor();
  testPushFront();
  testPushBack();
  testInsertIntoMiddle();
  testGetValue();
  testIsEmpty();
}



static void testCtor()
{
  //Конструктор по-умолчанию
  FineGrainedQueue testQueue_1;
  assert(testQueue_1.getSize() == 0);
  assert(testQueue_1.isEmpty() == true);

  //Параметризованный конструктор с инициализатором списка
  FineGrainedQueue testQueue_2 = {1,2,3,4,5};
  assert(testQueue_2.getSize() == 5);
  assert(testQueue_2.isEmpty() == false);
  assert(testQueue_2.getValue(0) == 1);
  assert(testQueue_2.getValue(1) == 2);
  assert(testQueue_2.getValue(2) == 3);
  assert(testQueue_2.getValue(3) == 4);
  assert(testQueue_2.getValue(4) == 5);
}



static void testPushFrontOnethread();
static void testPushFrontMiltithread();

static void testPushFront()
{
  testPushFrontOnethread();
  testPushFrontMiltithread();
}


static void testPushBackOnethread();
static void testPushBackMiltithread();

static void testPushBack()
{
  testPushBackOnethread();
  testPushBackMiltithread();
}



static void testInsertIntoMiddleOnethread();
static void testInsertIntoMiddleMiltithread();

static void testInsertIntoMiddle()
{
  testInsertIntoMiddleOnethread();
  testInsertIntoMiddleMiltithread();
}



static void testGetValue()
{
  FineGrainedQueue testQueue;

  testQueue.pushFront(1); //1
  assert(testQueue.getValue(0) == 1);

  testQueue.pushFront(2); //2 1
  assert(testQueue.getValue(0) == 2);
  assert(testQueue.getValue(1) == 1);

  testQueue.pushBack(3);  //2 1 3
  assert(testQueue.getValue(0) == 2);
  assert(testQueue.getValue(1) == 1);
  assert(testQueue.getValue(2) == 3);

  testQueue.pushBack(4);  //2 1 3 4
  assert(testQueue.getValue(0) == 2);
  assert(testQueue.getValue(1) == 1);
  assert(testQueue.getValue(2) == 3);
  assert(testQueue.getValue(3) == 4);

  //Тест выброс исключений
  // testQueue.getValue(11111); //pos is out_of_range
  // FineGrainedQueue testQueue_1;
  // testQueue_1.getValue(1); //list is empty
}



static void testIsEmpty()
{
  FineGrainedQueue testQueue_1;
  FineGrainedQueue testQueue_2;
  FineGrainedQueue testQueue_3;

  assert(testQueue_1.isEmpty() == true);
  assert(testQueue_2.isEmpty() == true);
  assert(testQueue_3.isEmpty() == true);

  //Любой метод, добавляющий элемент, делает список непустым
  testQueue_1.pushBack(1);
  testQueue_2.pushFront(0);
  testQueue_3.insertIntoMiddle(1,32);

  assert(testQueue_1.isEmpty() == false);
  assert(testQueue_2.isEmpty() == false);
  assert(testQueue_3.isEmpty() == false);
}



static void testPushFrontOnethread()
{
  FineGrainedQueue testQueue;
  for (size_t i=0; i<10; ++i){
    testQueue.pushFront(i);
    assert(testQueue.getValue(0) == static_cast<int>(i));
    assert(testQueue.getSize() == i+1);
  }
}



static void PushFrontForMiltithread(FineGrainedQueue& queue, int value);

static void testPushFrontMiltithread()
{
  for(size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_1 = 1;
    const int VALUE_2 = 2;
    const int VALUE_3 = 3;
    const int VALUE_4 = 4;
    std::thread thread_1(PushFrontForMiltithread, std::ref(testQueue), VALUE_1);
    std::thread thread_2(PushFrontForMiltithread, std::ref(testQueue), VALUE_2);
    std::thread thread_3(PushFrontForMiltithread, std::ref(testQueue), VALUE_3);
    std::thread thread_4(PushFrontForMiltithread, std::ref(testQueue), VALUE_4);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }
    if (thread_3.joinable()){
      thread_3.join();
    }
    if (thread_4.joinable()){
      thread_4.join();
    }
    assert(testQueue.getSize() == 4);
    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(0) == VALUE_1) ||
            (testQueue.getValue(0) == VALUE_2) ||
            (testQueue.getValue(0) == VALUE_3) ||
            (testQueue.getValue(0) == VALUE_4) );
    assert( (testQueue.getValue(1) == VALUE_1) ||
            (testQueue.getValue(1) == VALUE_2) ||
            (testQueue.getValue(1) == VALUE_3) ||
            (testQueue.getValue(1) == VALUE_4) );
    assert( (testQueue.getValue(2) == VALUE_1) ||
            (testQueue.getValue(2) == VALUE_2) ||
            (testQueue.getValue(2) == VALUE_3) ||
            (testQueue.getValue(2) == VALUE_4) );
    assert( (testQueue.getValue(3) == VALUE_1) ||
            (testQueue.getValue(3) == VALUE_2) ||
            (testQueue.getValue(3) == VALUE_3) ||
            (testQueue.getValue(3) == VALUE_4) );
  }
}



static void testPushBackOnethread()
{
  FineGrainedQueue testQueue;
  for (size_t i=0; i<10; ++i){
    testQueue.pushBack(i);
    assert(testQueue.getValue(testQueue.getSize()-1) == static_cast<int>(i));
    assert(testQueue.getSize() == i+1);
  }
}



static void PushBackForMiltithread(FineGrainedQueue& queue, int value);

static void testPushBackMiltithread()
{
  for(size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_1 = 1;
    const int VALUE_2 = 2;
    const int VALUE_3 = 3;
    const int VALUE_4 = 4;
    std::thread thread_1(PushBackForMiltithread, std::ref(testQueue), VALUE_1);
    std::thread thread_2(PushBackForMiltithread, std::ref(testQueue), VALUE_2);
    std::thread thread_3(PushBackForMiltithread, std::ref(testQueue), VALUE_3);
    std::thread thread_4(PushBackForMiltithread, std::ref(testQueue), VALUE_4);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }
    if (thread_3.joinable()){
      thread_3.join();
    }
    if (thread_4.joinable()){
      thread_4.join();
    }
    assert(testQueue.getSize() == 4);
    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(0) == VALUE_1) ||
            (testQueue.getValue(0) == VALUE_2) ||
            (testQueue.getValue(0) == VALUE_3) ||
            (testQueue.getValue(0) == VALUE_4) );
    assert( (testQueue.getValue(1) == VALUE_1) ||
            (testQueue.getValue(1) == VALUE_2) ||
            (testQueue.getValue(1) == VALUE_3) ||
            (testQueue.getValue(1) == VALUE_4) );
    assert( (testQueue.getValue(2) == VALUE_1) ||
            (testQueue.getValue(2) == VALUE_2) ||
            (testQueue.getValue(2) == VALUE_3) ||
            (testQueue.getValue(2) == VALUE_4) );
    assert( (testQueue.getValue(3) == VALUE_1) ||
            (testQueue.getValue(3) == VALUE_2) ||
            (testQueue.getValue(3) == VALUE_3) ||
            (testQueue.getValue(3) == VALUE_4) );
  }

  //Тест одновременно pushBack(), pushFront()
  for (size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_BACK = 11;
    const int VALUE_FRONT = 22;
    std::thread A(PushBackForMiltithread, std::ref(testQueue), VALUE_BACK);
    std::thread B(PushFrontForMiltithread, std::ref(testQueue), VALUE_FRONT);
    if (A.joinable()){
      A.join();
    }
    if (B.joinable()){
      B.join();
    }
    assert(testQueue.getSize() == 2);
    //Вне зависимости от порядка вызова потоков - порядок элементов неизменный
    assert(testQueue.getValue(0) == VALUE_FRONT);
    assert(testQueue.getValue(1) == VALUE_BACK);
  }
}



static void testInsertIntoMiddleOnethread()
{
  FineGrainedQueue testQueue;
  //Вставка в конец пустого списка
  testQueue.insertIntoMiddle(436, 99999); //436
  assert(testQueue.getValue(0) == 436);
  assert(testQueue.getSize() == 1);

  //Вставка в начало непустого списка
  testQueue.insertIntoMiddle(1, 0); //1 436
  assert(testQueue.getValue(0) == 1);
  assert(testQueue.getValue(1) == 436);
  assert(testQueue.getSize() == 2);

  //Вставка в конец непустого списка
  testQueue.insertIntoMiddle(2, 2); //1 436 2
  assert(testQueue.getValue(0) == 1);
  assert(testQueue.getValue(1) == 436);
  assert(testQueue.getValue(2) == 2);
  assert(testQueue.getSize() == 3);

  //Вставка в середину списка
  testQueue.insertIntoMiddle(3, 2); //1 436 3 2
  assert(testQueue.getValue(0) == 1);
  assert(testQueue.getValue(1) == 436);
  assert(testQueue.getValue(2) == 3);
  assert(testQueue.getValue(3) == 2);
  assert(testQueue.getSize() == 4);

  testQueue.insertIntoMiddle(-1, 1); //1 -1 436 3 2
  assert(testQueue.getValue(0) == 1);
  assert(testQueue.getValue(1) == -1);
  assert(testQueue.getValue(2) == 436);
  assert(testQueue.getValue(3) == 3);
  assert(testQueue.getValue(4) == 2);
  assert(testQueue.getSize() == 5);

  FineGrainedQueue testQueue_1;
  //Вставка в начало пустого списка
  testQueue_1.insertIntoMiddle(1,0);
  assert(testQueue_1.getValue(0) == 1);
  assert(testQueue_1.getSize() == 1);
}



static void PushInsertIntoMiddleMiltithread(FineGrainedQueue& queue,
                                            int value,
                                            size_t pos);

static void testInsertIntoMiddleMiltithread()
{
  //Вставка в КОНЕЦ ПУСТОГО списка
  for (size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_1 = 1;
    const int VALUE_2 = 2;
    const int VALUE_3 = 3;
    const int VALUE_4 = 4;
    //Вставка в конец - позиция > size
    const size_t POS = 99999;
    std::thread thread_1(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_1,
                          POS);
    std::thread thread_2(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_2,
                          POS);
    std::thread thread_3(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_3,
                          POS);
    std::thread thread_4(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_4,
                          POS);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }
    if (thread_3.joinable()){
      thread_3.join();
    }
    if (thread_4.joinable()){
      thread_4.join();
    }
    assert(testQueue.getSize() == 4);
    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(0) == VALUE_1) ||
            (testQueue.getValue(0) == VALUE_2) ||
            (testQueue.getValue(0) == VALUE_3) ||
            (testQueue.getValue(0) == VALUE_4) );
    assert( (testQueue.getValue(1) == VALUE_1) ||
            (testQueue.getValue(1) == VALUE_2) ||
            (testQueue.getValue(1) == VALUE_3) ||
            (testQueue.getValue(1) == VALUE_4) );
    assert( (testQueue.getValue(2) == VALUE_1) ||
            (testQueue.getValue(2) == VALUE_2) ||
            (testQueue.getValue(2) == VALUE_3) ||
            (testQueue.getValue(2) == VALUE_4) );
    assert( (testQueue.getValue(3) == VALUE_1) ||
            (testQueue.getValue(3) == VALUE_2) ||
            (testQueue.getValue(3) == VALUE_3) ||
            (testQueue.getValue(3) == VALUE_4) );
  }

  //Вставка в НАЧАЛО НЕпустого списка
  for (size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_INI = 1;
    testQueue.pushBack(VALUE_INI);

    const int VALUE_1 = 11;
    const int VALUE_2 = 21;
    //Вставка в начало
    const size_t POS = 0;
    std::thread thread_1(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_1,
                          POS);
    std::thread thread_2(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_2,
                          POS);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }
    assert(testQueue.getSize() == 3);
    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(0) == VALUE_1) ||
            (testQueue.getValue(0) == VALUE_2) );
    //Последний элемент точно определён
    assert(testQueue.getValue(2) == VALUE_INI);
  }

  //Вставка в КОНЕЦ НЕпустого списка
  for (size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_INI = 1;
    testQueue.pushBack(VALUE_INI);

    const int VALUE_1 = 11;
    const int VALUE_2 = 21;
    //Вставка в конец - позиция > size
    const size_t POS = 99999;
    std::thread thread_1(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_1,
                          POS);
    std::thread thread_2(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_2,
                          POS);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }
    assert(testQueue.getSize() == 3);
    //Первый элемент точно определён
    assert(testQueue.getValue(0) == VALUE_INI);
    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(1) == VALUE_1) ||
            (testQueue.getValue(1) == VALUE_2) );
    assert( (testQueue.getValue(2) == VALUE_1) ||
            (testQueue.getValue(2) == VALUE_2) );
  }

  //Вставка в СЕРЕДИНУ списка
  for (size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_INI_1 = 1;
    const int VALUE_INI_2 = 2;
    const int VALUE_INI_3 = 3;
    testQueue.pushBack(VALUE_INI_1);
    testQueue.pushBack(VALUE_INI_2);
    testQueue.pushBack(VALUE_INI_3);
    //testQueue: 1 2 3
    const int VALUE_1 = 11;
    const int VALUE_2 = 21;
    //Вставить 2-м (вторым) элементом
    const size_t POS = 1;
    std::thread thread_1(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_1,
                          POS);
    std::thread thread_2(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_2,
                          POS);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }

    assert(testQueue.getSize() == 5);
    //Первый элемент точно определён
    assert(testQueue.getValue(0) == VALUE_INI_1);
    //Последний элемент точно определён
    assert(testQueue.getValue(4) == VALUE_INI_3);

    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(POS) == VALUE_1) ||
            (testQueue.getValue(POS) == VALUE_2) );
  }

  //Вставка в начало пустого списка
  for (size_t i=0; i<100; ++i){
    FineGrainedQueue testQueue;
    const int VALUE_1 = 11;
    const int VALUE_2 = 21;
    const size_t POS = 0;
    std::thread thread_1(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_1,
                          POS);
    std::thread thread_2(PushInsertIntoMiddleMiltithread,
                          std::ref(testQueue),
                          VALUE_2,
                          POS);
    if (thread_1.joinable()){
      thread_1.join();
    }
    if (thread_2.joinable()){
      thread_2.join();
    }

    assert(testQueue.getSize() == 2);
    //Порядок вызова потоков недетерминирован - важно само наличие тестовых чисел
    assert( (testQueue.getValue(0) == VALUE_1) ||
            (testQueue.getValue(0) == VALUE_2) );
  }
}



static void PushFrontForMiltithread(FineGrainedQueue& queue, int value)
{
  queue.pushFront(value);
}



static void PushBackForMiltithread(FineGrainedQueue& queue, int value)
{
  queue.pushBack(value);
}



static void PushInsertIntoMiddleMiltithread(FineGrainedQueue& queue,
                                            int value,
                                            size_t pos)
{
  queue.insertIntoMiddle(value, pos);
}