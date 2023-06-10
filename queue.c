#include "queue.h"
#include <threads.h>


void lock_acquire(void);
void lock_release(void);

typedef struct thread_node{
    cnd_t cv;
    int cvi;
    struct thread_node* next;
} thread_node;

typedef struct thread_LL{
    thread_node* first;
    thread_node* last;
    size_t waiting;
} thread_LL;

typedef struct item_node{
    void* item;
    struct item_node* next;
} item_node;

typedef struct item_LL{
    item_node* first;
    item_node* last;
    size_t size;
    size_t visited;
} item_LL;

int init = 1;


item_LL* item_Q;
mtx_t lock;
thread_LL* thread_Q;

// initialize the queue
void initQueue(void){
    printf("Qs initialization starting\n");
    if(init == 0){
        printf("The queue has already been initialized\n");
        return;
    }

    item_Q = (item_LL*) malloc(sizeof(item_LL));
    int rc = mtx_init(&lock, mtx_plain);
    if(rc != thrd_success){
        printf("Failed to initialize the queue lock\n");
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
    printf("Qs initialization finished\n");
}

// destroy the queue
void destroyQueue(void) {
    printf("Qs destroyment starting\n");
    lock_acquire();
    if(init == 1){
        printf("The queue has not been initialized\n");
        lock_release();
        return;
    }

    if(waiting() > 0) { 
        printf("There are waiting threads!\n");
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
    
    printf("Qs destroyed\n");
    return;    
}

// add an item to the queue
void enqueue(void* item) {
    printf("Enqueueing an item\n");
    lock_acquire();
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

    if(waiting() == 0){ // if there is no waiting thread, return
        printf("error 1==============\n");
        lock_release();
        return;
    }
    else{
        printf("the condition variable number sent is:  %d\n", (thread_Q->first->cvi));

        cnd_signal(&(thread_Q->first->cv));
        

    }
    

    printf("finished enqueueing item\n");

    return;

}

// remove an item from the queue
void* dequeue(void){
    lock_acquire();
    printf("number of item is: %ld\n", size());
    if(size() > 0){
        item_node* temp = item_Q->first;
        item_Q->first = item_Q->first->next;
        if(item_Q->first == NULL){
            item_Q->last = NULL;
        }
        item_Q->size--;
        void* item = temp->item;
        free(temp);
        item_Q->visited++;
        lock_release();
        printf("Finished dequeueing exit 1");
        return item;
    }
    else{
        thread_node* node = (thread_node*) malloc(sizeof(thread_node));
        cnd_init(&(node->cv));
        node->cvi = waiting();
        node->next = NULL;
        if(thread_Q->last == NULL){
            thread_Q->last = node;
            thread_Q->first = node;
        }
        else{ // if there is no items 
            thread_node* temp = thread_Q->last;
            thread_Q->last = node;
            temp->next = node;
        }
        thread_Q->waiting++;

        printf("number of waiting threads is :%ld\n", waiting());
        printf("the condition variable number waiting for is:  %d\n", (thread_Q->first->cvi));


        cnd_wait(&(node->cv), &lock); // wait for an item to become available
        printf("cnd wait got the right signal");
        fflush(stdout);
        cnd_destroy(&(node->cv));


        void* res = item_Q->first->item;
        item_node* tmp1 = item_Q->first;
        if(item_Q->first->next == NULL){
            item_Q->last = NULL;
        }
        else{
            item_Q->first = item_Q->first->next;    
        }
        free(tmp1);


        thread_node* tmp2 = thread_Q->first;
        if(thread_Q->first == NULL){
            thread_Q->last = NULL;
        }
        else{
            thread_Q->first = thread_Q->first->next;
        }
        free(tmp2);

        thread_Q->waiting--;
        item_Q->visited++;
        lock_release();
        printf("Finished dequeueing exit 2");
        
        return res;
    }
     
}


// try to remove an item from the queue
bool tryDequeue(void** arg){
    printf("trying to dequeue\n");
    lock_acquire();
    if( (waiting() != 0) || size() == 0){ // if dequeue is unavailable
        lock_release();
        printf("dequeue not aviable\n");
        return false;
    }

    item_node* temp = item_Q->first;  // if dequeue is available
    item_Q->first = item_Q->first->next;
    if(item_Q->first == NULL){
        item_Q->last = NULL;
    }
    item_Q->size--;
    *arg = temp->item; // im not sure if this is right
    free(temp);
    item_Q->visited++;
    lock_release();
    printf("finished dequeue trying succ\n");
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

