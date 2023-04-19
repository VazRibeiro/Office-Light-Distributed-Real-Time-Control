#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

template <class T, int buffer_size = 6000>
class circular_buffer {

    T buffer[buffer_size]; //storage
    int head; //index of next element to write
    int tail; //index of next element to read
    int count; //counter of valid elements in the buffer
    

    public:

    circular_buffer() : head {0}, tail{0}, count{0} {};
    ~circular_buffer() {};
    int capacity() const { return buffer_size; }
    int size() const { return count; }
    bool is_empty() { return count == 0; }
    bool is_full() { return count == buffer_size; }
    T peak() { return buffer[tail]; } // inspect but keep value invalid if buffer empty
    int getHead() const { return head; }

    void put(T elem) {
        buffer[head] = elem;
        head = (head + 1) % buffer_size;
        if (count == buffer_size) {
            tail = (tail + 1) % buffer_size;
        } else {
            count++;
        }
    } //update head with wraparound if necessary, also update tail if buffer already full

    T take() {//remove an element from the buffer
        T tmp_tail = buffer[tail]; //invalid if buffer empty
        if(!is_empty()){
            count--; tail++;
            if(tail == buffer_size) tail = 0;
        }
        return tmp_tail;
    }

    T read(int index) const {
        return buffer[index];
    }

    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
};

#endif //CIRCULAR_BUFFER_H