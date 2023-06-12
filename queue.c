#include "queue.h"
#include <threads.h>


void lock_acquire(void);
void lock_release(void);

typedef struct thread_node{ // struct to represent a thread
    cnd_t cv;   
    int channel;
    struct thread_node* next;
} thread_node;

typedef struct thread_LL{ // struct to represent a linked list of threads
    thread_node* first;
    thread_node* last;
    _Atomic size_t waiting;
    int max_channel;
} thread_LL;

typedef struct item_node{ // struct to represent an item
    void* item;
    struct item_node* next;
    int channel;

} item_node;

typedef struct item_LL{ // struct to represent a linked list of items
    item_node* first;
    item_node* last;
    int max_channel;
    _Atomic size_t size;
    _Atomic size_t visited;
} item_LL;

int init = 1;


item_LL* item_Q;
mtx_t lock;
thread_LL* thread_Q;

// initialize both of the queues
void initQueue(void){
    if(init == 0){
        return;
    }

    item_Q = (item_LL*) malloc(sizeof(item_LL));
    int rc = mtx_init(&lock, mtx_plain);
    if(rc != thrd_success){
        return;
    }
    item_Q->first = NULL;
    item_Q->last = NULL;
    item_Q->visited = 0;
    item_Q->size = 0;
    item_Q->max_channel = 0;

    thread_Q = (thread_LL*) malloc(sizeof(thread_LL));
    thread_Q->first = NULL;
    thread_Q->last = NULL;
    thread_Q->waiting = 0;
    thread_Q->max_channel = 0;

    init = 0;
}

// destroy bot of the queues
void destroyQueue(void) {
    lock_acquire();
    if(init == 1){
        lock_release();
        return;
    }

    if(waiting() > 0) { 
    }

    item_node* temp1 = item_Q->first;
    while(temp1 != NULL){

        item_node* next1 = temp1->next;
        free(temp1);
        temp1 = next1;
    }
    thread_node* temp2 = thread_Q->first;
    while (temp2 != NULL){
        thread_node* next2 = temp2;
        free(temp2);
        temp2 = next2;        
    }
    
    free(thread_Q);
    free(item_Q);
    init = 1;
    mtx_destroy(&lock);
    
    return;    
}

// add an item to the queue
void enqueue(void* item) {
    lock_acquire();
    item_node* new_node = malloc(sizeof(item_node));
    new_node->item = item;
    new_node->next = NULL;
    new_node->channel = item_Q->max_channel; // for each item sets a diff channel number that will match the right thread channel number
    item_Q->max_channel++;
    

    if(item_Q->first == NULL){
        item_Q->first = new_node;
        item_Q->last = new_node;
    }
    else{
        item_Q->last->next = new_node;
        item_Q->last = new_node;
    }
    
    
    if(waiting() <= size()){ // if there is no waiting thread, return
        item_Q->size++;
        lock_release();
        return;
    }
    else{ // else, wake up the first waiting thread
        item_Q->size++;
        cnd_signal(&(thread_Q->first->cv));

        thread_node* tmp2 = thread_Q->first;  // delete the thread node that was waked up, so if another enqueue will be called, it will not wake up the same thread
        if (thread_Q->first->next == NULL) {
            thread_Q->last = NULL;
            thread_Q->first = NULL;
        }
        else{
            thread_Q->first = thread_Q->first->next;
        }
        free(tmp2);

        lock_release();
    }

    return;

}

// remove an item from the queue
void* dequeue(void){
    lock_acquire();

    if(size() > waiting()){ // if there is more items than waiting threads, remove the first item
        int channel = thread_Q->max_channel;
        thread_Q->max_channel ++;
        item_node* temp = item_Q->first;

        if(temp->channel == channel){ // the right one is in the first one in the Q
            item_Q->first = item_Q->first->next;
        }
        else{
            item_node* temp2 = temp->next;
            while(temp2->channel != channel){ // search for the right item
                temp = temp->next;
                temp2 = temp->next;
            }
            if(temp2 == item_Q->last){ // the right one is the last one in the Q
                item_Q->last = temp;
                
            }
            else{
                temp->next = temp2->next; // the right one not in the last one in the Q
            }
            temp = temp2;
        }

        if(item_Q->first == NULL){
            item_Q->last = NULL;
        }
        item_Q->size--;
        void* item = temp->item;
        free(temp);
        item_Q->visited++;

        lock_release();
        return item;
    }
    else{ // there is no items in the Q, so wait for an item to be added
        thread_node* node = (thread_node*) malloc(sizeof(thread_node)); 
        cnd_init(&(node->cv));
        node->next = NULL;
        node->channel = thread_Q->max_channel ;
        thread_Q->max_channel ++;
        if(thread_Q->last == NULL){
            thread_Q->last = node;
            thread_Q->first = node;
        }
        else{ // if there is no threads
            thread_Q->last->next = node;
            thread_Q->last = node;            
        }
        thread_Q->waiting++;



        while(size() == 0) {
            cnd_wait(&(node->cv), &lock); // wait for an item to become available
        }
        
        cnd_destroy(&(node->cv));

        
        void* res = item_Q->first->item; // removes the item from the Q
        item_node* tmp1 = item_Q->first;
        if(item_Q->first->next == NULL){
            item_Q->last = NULL;
            item_Q->first = NULL;
        }
        else{
            item_Q->first = item_Q->first->next;    
        }
        free(tmp1);




        thread_Q->waiting--;
        item_Q->visited++;
        item_Q->size--;
        lock_release();
        
        return res;
    }
     
}

// try to remove an item from the queue
bool tryDequeue(void** arg){ 
    lock_acquire();
    if( size() <= waiting()) { // this if checks that there is no way for a tryDequeue to succeed, both
        lock_release();
        return false;
    }
    int channel = thread_Q->max_channel;
    thread_Q->max_channel ++;
    item_node* temp = item_Q->first;
    
    if(temp->channel == channel){ // the right one is in the first one in the Q
        
        item_Q->first = item_Q->first->next;
        
    }

    else{
        
        item_node* temp2 = temp->next;
        while(temp2->channel != channel){
            temp = temp->next;
            temp2 = temp->next;
        }
        if(temp2 == item_Q->last){ // the right one is the last one in the Q
            item_Q->last = temp;    
        }
        else{
            temp->next = temp2->next; // the right one not in the last one in the Q
        }
        temp = temp2;
    }

    if(item_Q->first == NULL){
        item_Q->last = NULL;
    }
    item_Q->size--;
    *arg = temp->item;

    free(temp);
    item_Q->visited++;

    lock_release();
    return true;
}

// return the size of the queue
size_t size(void){return item_Q->size;}

// return the number of waiting threads
size_t waiting(void){return thread_Q->waiting;}

// return the number of items that have been removed from the queue
size_t visited(void){return item_Q->visited;}

// acquire the lock
void lock_acquire(void){while(mtx_lock(&lock) < 0){}}

// release the lock
void lock_release(void){mtx_unlock(&lock);}
