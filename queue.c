#include "queue.h"
#include <threads.h>

typedef struct thread_node{ // struct to represent a thread
    cnd_t cv;   
    struct thread_node* next;
    void* item;
} thread_node;

typedef struct thread_LL{ // struct to represent a linked list of threads
    thread_node* first;
    thread_node* last;
    _Atomic size_t waiting;
} thread_LL;

typedef struct item_node{ // struct to represent an item
    void* item;
    struct item_node* next;

} item_node;

typedef struct item_LL{ // struct to represent a linked list of items
    item_node* first;
    item_node* last;
    _Atomic size_t size;
    _Atomic size_t visited;
} item_LL;


void lock_acquire(void);
void lock_release(void);
void print_itmes(void);
void delete_thread_node(thread_node* node);
void delete_item_node(item_node* node);



int init = 1;
int p = 0;

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

    thread_Q = (thread_LL*) malloc(sizeof(thread_LL));
    thread_Q->first = NULL;
    thread_Q->last = NULL;
    thread_Q->waiting = 0;

    init = 0;
}

// destroy bot of the queues
void destroyQueue(void) {
    lock_acquire();
    if(init == 1){
        lock_release();
        return;
    }

    item_node* node = item_Q->first;
    while(node != NULL){
        item_node* temp = node;
        node = node->next;
        free(temp);
    }

    thread_node* thread = thread_Q->first;
    while(thread != NULL){
        thread_node* temp = thread;
        thread = thread->next;
        free(temp);
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
    if(p){
        printf("enqueue\n");
        print_itmes();
    }
    
    if(waiting() <= size()){ // if there is no waiting thread, return
        item_node* new_node = malloc(sizeof(item_node));
        new_node->item = item;
        new_node->next = NULL;
        if(item_Q->first == NULL){
            item_Q->first = new_node;
            item_Q->last = new_node;
        }
        else{
            item_Q->last->next = new_node;
            item_Q->last = new_node;
        }   
        item_Q->size++;
        lock_release();
        return;
        }
    else{ // else, wake up the first waiting thread
        item_Q->size++;

        thread_node* thread = thread_Q->first;  // delete the thread node that was waked up, so if another enqueue will be called, it will not wake up the same thread
        
        thread->item = item;
        
        delete_thread_node(thread);


        cnd_signal(&(thread->cv));

        

        lock_release();
    }

    return;

}

// remove an item from the queue
void* dequeue(void){
    lock_acquire();
    if(p){
        printf("dequeue\n");
        print_itmes();
    }
    if(size() > waiting()){ // if there is more items than waiting threads, remove the first item

        item_node* temp = item_Q->first;
        void* item = temp->item;

        item_Q->first = item_Q->first->next;
        if(item_Q->first == NULL){
            item_Q->last = NULL;
        }

        item_Q->size--;
        free(temp);
        item_Q->visited++;

        lock_release();
        return item;
    }
    else{ // there is no items in the Q, so wait for an item to be added
        thread_node* node = (thread_node*) malloc(sizeof(thread_node)); 
        cnd_init(&(node->cv));
        node->next = NULL;

        if(thread_Q->last == NULL){ // if there are no threads
            thread_Q->last = node;
            thread_Q->first = node;
        }
        else{ 
            thread_Q->last->next = node;
            thread_Q->last = node;            
        }
        thread_Q->waiting++;

        while(size() == 0) {
            cnd_wait(&(node->cv), &lock); // wait for an item to become available
        }
        if(p){
            printf("got signal\n");
            print_itmes();
        }
        cnd_destroy(&(node->cv));
        
        void* res = node->item;


        thread_Q->waiting--;
        item_Q->visited++;
        item_Q->size--;
        free(node);
        lock_release();
        
        return res;
    }
     
}

// try to remove an item from the queue
bool tryDequeue(void** arg){ 
    lock_acquire();
    if(p){
        printf("tryDequeue\n");
        print_itmes();
    }
    if( size() <= waiting()) { // this if checks that there is no way for a tryDequeue to succeed, both
        lock_release();
        return false;
    }

    *arg = item_Q->first->item;
    
    item_node* node = item_Q->first;

    delete_item_node(node);

    item_Q->visited++;
    item_Q->size--;
    
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

void print_itmes(){
    /*item_node* temp = item_Q->first;
    printf("=========================");
    printf("the items are:\n");
    while(temp != NULL){
        if(temp->item == NULL){
            printf("stoped\n");
            break;
        }
        printf("%d\n", *(int*)temp->item);
        temp = temp->next;
    }
    printf("=========================");*/
    return;
}


void delete_thread_node(thread_node* node){
    if (node == thread_Q->first){
        if (node->next == NULL){   // if there is only one thread in the Q
            thread_Q->first = NULL;
            thread_Q->last = NULL;
        }
        else{                      // if there is more than one thread in the Q
            thread_Q->first = node->next;
        }
    }
    else{
        thread_node* prev = thread_Q->first;
        while(prev->next != node){ // ##############################################
            prev = prev->next;
        }
        if (node == thread_Q->last){  // if the node is the last one in the Q
            thread_Q->last = prev;
        }
        prev->next = node->next;
    }
}

void delete_item_node(item_node* node){
    if (node == item_Q->first){
        if (node->next == NULL){   // if there is only one item in the Q
            item_Q->first = NULL;
            item_Q->last = NULL;
        }
        else{                      // if there is more than one item in the Q
            item_Q->first = node->next;
        }
        
    }
    else{
        item_node* prev = item_Q->first;
        while(prev->next != node){
            prev = prev->next;
        }
        if (node == item_Q->last){  // if the node is the last one in the Q
            item_Q->last = prev;
        }
        prev->next = node->next;
    }
    free(node);
}
