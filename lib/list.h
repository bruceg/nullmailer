#ifndef LIST__H__
#define LIST__H__

template<class T> struct list_node
{
  list_node* next;
  T data;
  list_node(T d, list_node* n = 0) : next(n), data(d) { }
  ~list_node() { }
};

template<class T> class list_iterator;
template<class T> class const_list_iterator;

template<class T> class list
{
public:
  typedef list_node<T> node;
  typedef list_iterator<T> iter;
  typedef const_list_iterator<T> const_iter;
  friend class list_iterator<T>;
  friend class const_list_iterator<T>;
  
  list()
    : head(0), tail(0), cnt(0)
    {
    }
  list(const list&);
  
  ~list()
    {
      empty();
    }

  unsigned count() const
    {
      return cnt;
    }

  void empty()
    {
      while(head) {
	node* next = head->next;
	delete head;
	head = next;
      }
      tail = 0;
      cnt = 0;
    }
  
  bool append(T data) 
    {
      node* n = new node(data);
      if(tail)
	tail->next = n;
      else
	head = n;
      tail = n;
      ++cnt;
      return true;
    }
  bool prepend(T data)
    {
      head = new node(data, head);
      if(!tail)
	tail = head;
      ++cnt;
      return true;
    }
  bool remove(iter&);
private:
  node* head;
  node* tail;
  unsigned cnt;
};

template<class T> class const_list_iterator
{
  friend class list<T>;
private:
  inline void go_next()
    {
      prev = curr;
      if(curr)
	curr = curr->next;
    }
public:
  const_list_iterator(const list<T>& l)
    : lst(l), prev(0), curr(l.head)
    {
    }
  void operator++()
    {
      go_next();
    }
  void operator++(int) 
    {
      go_next();
    }
  T operator*() const
    {
      return curr->data;
    }
  bool operator!() const
    {
      return curr == 0;
    }
  operator bool() const
    {
      return !operator!();
    }
private:
  const list<T>& lst;
  // g++ 3.2 insists
  const typename list<T>::node* prev;
  const typename list<T>::node* curr;
};

template<class T>
list<T>::list(const list<T>& that)
  : head(0), tail(0), cnt(0)
{
  for(const_iter i = that; i; ++i)
    append(*i);
}

template<class T> class list_iterator
{
  friend class list<T>;
private:
  inline void go_next()
    {
      prev = curr;
      if(curr)
	curr = curr->next;
    }
public:
  list_iterator(list<T>& l)
    : lst(l), prev(0), curr(l.head)
    {
    }
  void operator++()
    {
      go_next();
    }
  void operator++(int) 
    {
      go_next();
    }
  T operator*() const
    {
      return curr->data;
    }
  T& operator*()
    {
      return curr->data;
    }
  bool operator!() const
    {
      return curr == 0;
    }
  operator bool() const
    {
      return !operator!();
    }
private:
  list<T>& lst;
  typename list<T>::node* prev;
  typename list<T>::node* curr;
};

template<class T>
bool list<T>::remove(list<T>::iter& iter)
{
  if(this != &iter.lst)
    return false;
  if(!iter.curr)
    return false;
  if(iter.prev) {
    iter.prev->next = iter.curr->next;
    if(iter.curr == tail)
      tail = iter.prev;
    delete iter.curr;
    iter.curr = iter.prev->next;
  }
  else {
    head = iter.curr->next;
    if(!head)
      tail = 0;
    delete iter.curr;
    iter.curr = head;
  }
  --cnt;
  return true;
}

#endif // LIST__H__
