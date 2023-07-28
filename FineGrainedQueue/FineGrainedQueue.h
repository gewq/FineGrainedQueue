/**
\file FineGrainedQueue.h
\brief Класс - потокобезопасный односвязный список с мелкогранулярными блокировками

Методы:
- добавить элемент в начало списка
- добавить элемент в конец списка
- добавить элемент в заданную позицию списка
- получить количество элементов в списке
- получить значение элемента в заданной позиции списка
- получить признак - пуст ли список
*/

#pragma once

#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <memory>
#include <initializer_list>


class FineGrainedQueue{
  public:
    //Элемент списка
    struct Node{
      explicit Node(int v): value(v), next(nullptr){}
      int value;
      std::shared_ptr<Node> next;
      std::shared_mutex mutex;
    };

    FineGrainedQueue();
		FineGrainedQueue(std::initializer_list<int> values);

    FineGrainedQueue(const FineGrainedQueue& other) = delete;
    FineGrainedQueue(const FineGrainedQueue&& other) = delete;
    FineGrainedQueue& operator=(const FineGrainedQueue& other) = delete;
    FineGrainedQueue& operator=(FineGrainedQueue&& other) = delete;

    ~FineGrainedQueue();

    /**
    Вставить элемент в начало списка
    \param[in] value Значение элемента
    */
    void pushFront(int value);

    /**
    Вставить элемент в конец списка
    \param[in] value Значение элемента
    */
    void pushBack(int value);

    /**
    Вставить элемент в заданную позицию
    Если позиция больше длины списка - вставить в конец
    \param[in] value Значение элемента
    \param[in] pos Позиция в списке куда поместить
    */
    void insertIntoMiddle(int value, size_t pos);

    /**
    \return Количество элементов списка
    */
    size_t getSize() const;

    /**
    \param[in] pos Позиция в списке
    \return Значение элемента
    */
    int getValue(size_t pos) const;

    /**
    \return Признак пуст ли список
    */
    bool isEmpty() const;

  private:
    std::atomic<size_t> size_;    //Размер списка
    std::shared_ptr<Node> head_;  //Указатель на первый элемент
    std::shared_ptr<Node> tail_;  //Указатель на последний элемент
    mutable std::shared_mutex mutexHead_;
    mutable std::shared_mutex mutexTail_;
};



namespace fine_grained_queue{
  /**
  Протестировать публичные методы класса
  */
  void test();
}