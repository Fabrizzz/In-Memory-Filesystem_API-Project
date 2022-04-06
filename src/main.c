#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define REALLOC_STEP 255
#define MAX_HEIGHT 255
#define MAX_SONS 1024
#define RESIZE_FACTOR 2
#define HASH_TABLE_INITIAL_SIZE 4096

/**
 * List of valid commands:
 * create /file_path
 * create_dir /directory_path
 * read /file_path
 * write /file_path "content"
 * delete /file_path
 * delete /directory_path
 * delete_r /file_path
 * delete_r /directory_path
 * find resource_name
 */

/**
 * A node represents a Resource of the filesystem: either a file or a directory
 */
typedef struct t {
    bool type;                //Type=0 directory || type=1 file
    char *path;               //String representing the path of the resource
    char *data;               //String content of the file, null in case of a directory or an empty file
    struct t *next;           //Next directory or file, that is in the same directory as this
    struct t *son;            //First of the directories or files contained into this
    short int height;         //The root that represents the empty path has height == 0
    short int number_of_sons; //Number of directories or files contained into this, limited by MAX_SONS
} Node;

/**
 * Node of a linkedList, used to implement the find()
 */
typedef struct n {
    Node *current;
    struct n *next;
} Linked_node;


void init();                          //initialize root Node and the empty Hash Table
Node *create_node();                  //create an empty Node
char *get_input(int realloc_step);    //gets a String from STDIN
void create(char *path, char *input); //create a file or a directory
Node *find(char *str);                //find the Node associated to a Path, or NULL if absent, using an hashMap
void add_to_hashMap(Node *node);      //add a Node into the hashMap
void read(char *curr);                //prints the content of a file on STDOUT
void write(char *curr);               //overwrites the content of a file
int delete(char *curr);               //deletes a Node, only if it doesn't have sons
void hash_delete(Node *old);          //deletes a Node from the hashmap
void findAll(char *curr);             //recursive find, returns all the elements in topological order
void delete_r(char *curr);            //deletes an element and all of his sons
void increase_hashMap();              //increases the capacity of the Hashmap and recomputes all the HashCode
void recompute_hashes(Node *el);      //recomputes all the HashCodes of the nodes


uint32_t jenkins_one_at_a_time_hash(char *key, size_t length);  //Hashing function
int cstring_cmp(const void *a, const void *b);                  //string compare function


Node *root;                                    //Root of the tree, representing the empty path "/"
Node **hash_table;                             //Global Hash Table containing every node
int hash_table_size = HASH_TABLE_INITIAL_SIZE; //Hash Table current size
int global_counter = 0;                        //Total number of nodes


int main() {
    char *input;
    char *token;  //token obtained by the strtok

    init();

    while (true) {
        input = get_input(REALLOC_STEP);
        token = strtok(input, " ");

        if (!strcmp(token, "create_dir") || !strcmp(token, "create")) {
            token = strtok(NULL, " "); //path
            create(token, input);
            if (global_counter == (hash_table_size / RESIZE_FACTOR))
                increase_hashMap();
        } else if (!strcmp(token, "read")) {
            token = strtok(NULL, " ");
            read(token);
        } else if (!strcmp(token, "write")) {
            token = strtok(NULL, " ");
            write(token);
        } else if (!strcmp(token, "delete")) {
            token = strtok(NULL, " ");
            if (delete(token))
                printf("ok\n");
            else
                printf("no\n");
        } else if (!strcmp(token, "delete_r")) {
            token = strtok(NULL, " ");
            delete_r(token);
        } else if (!strcmp(token, "find")) {
            token = strtok(NULL, " ");
            findAll(token);
        } else if (!strcmp(token, "exit")) {
            free(input);
            return 0;
        }
        free(input);
    }
}

/**
 * It initializes the empty Hash Table and creates the first Root node of the tree;
 */
void init() {
    root = create_node();
    hash_table = (Node **) calloc(hash_table_size, sizeof(Node *));
}

/**
 * It creates an empty node, that represents a directory by default
 */
Node *create_node() {
    Node *node;

    node = (Node *) malloc(sizeof(Node));
    node->type = 0;                  //by default, 0 == directory
    node->path = NULL;
    node->data = NULL;
    node->next = NULL;
    node->son = NULL;
    node->height = 0;
    node->number_of_sons = 0;

    return node;
}

/**
 * It returns a char array containing a string taken from STDIN, of the exact size of the input
 */
char *get_input(int realloc_step) {
    char *str;
    int ch;
    int pos = 0;
    str = realloc(NULL, sizeof(char) * (realloc_step));
    while (EOF != (ch = fgetc(stdin)) && ch != '\n') {
        str[pos++] = ch;
        if (pos == realloc_step) {
            str = realloc(str, sizeof(char) * (realloc_step += REALLOC_STEP));
        }
    }
    str[pos++] = '\0';

    return realloc(str, sizeof(char) * pos);
}

/**
 * It creates a node on the given path, sets it as a file or as a directory, depending on the command in input,
 * and adds it to the global HashMap
 * It fails if a node with the given path already exists, or if the directory in which the new node should be placed
 * doesn't exists or if the maximum height of the three is violated
 */
void create(char *path, char *input) {
    Node *parent;
    Node *son;
    char *last_slash;

    son = find(path);
    last_slash = strrchr(path, '/');
    *last_slash = '\0';
    parent = find(path);
    *last_slash = '/';

    if (!parent || son) //check that the parent directory exists and that the current directory doesn't already exists
        printf("no\n");
    else {
        //check that the parent is a directory and not a file and that the maximum height is not violated
        if (parent->type == 1 || parent->height >= MAX_HEIGHT) {
            printf("no\n");
        } else {
            if (parent->number_of_sons == MAX_SONS) {
                printf("no\n");
                return;
            }
            son = create_node();
            if (parent->son != NULL) {
                son->next = parent->son;
            }
            parent->son = son;
            son->path = (char *) malloc((strlen(path) + 1) * sizeof(char));
            strcpy(son->path, path);
            son->height = (parent->height) + 1;
            if (!strcmp(input, "create"))
                son->type = 1;
            parent->number_of_sons++;
            add_to_hashMap(son);
            global_counter++;
            printf("ok\n");
        }
    }
}
/**
 * It uses the Global Hash Map to find a Node at the given Path, or returns NULL if it is not find
 */
Node *find(char *str) {
    unsigned int temp;
    if (str[0] == '\0') {
        return root;
    }
    temp = jenkins_one_at_a_time_hash(str, strlen(str));
    while (hash_table[temp] != NULL) {
        if (!strcmp(str, hash_table[temp]->path))
            return hash_table[temp];
        temp = (temp + 1) % hash_table_size;
    }
    return NULL;
}

/**
 * It adds the new node to the global HashMap
 */
void add_to_hashMap(Node *node) {
    unsigned int temp;
    temp = jenkins_one_at_a_time_hash(node->path, strlen(node->path));
    while (hash_table[temp] != NULL)
        temp = (temp + 1) % hash_table_size;
    hash_table[temp] = node;
}

/**
 * It reads the content of the current file, or prints "no" if the current Path doesn't exists, or it isn't a file
 */
void read(char *curr) {
    Node *temp;
    temp = find(curr);
    if (temp == NULL || temp->type == 0)
        printf("no\n");
    else if (temp->data == NULL)
        printf("contenuto \n");
    else
        printf("contenuto %s\n", temp->data);
}

/**
 * It overrides the content of a file with the Input given
 * It prints "no" if the current Path doesn't exists, or if it is not a file
 */
void write(char *curr) {
    int len;
    Node *currentNode;
    currentNode = find(curr);
    if (currentNode == NULL || currentNode->type == 0)
        printf("no\n");
    else {
        if (currentNode->data != NULL)
            free(currentNode->data);
        curr = strtok(NULL, " ");
        len = strlen(curr);
        curr[len - 1] = '\0'; //eliminates the " at the end of the string
        curr = &curr[1]; //eliminates the " at the beginning of the string
        currentNode->data = (char *) malloc((len - 1) * sizeof(char));
        strcpy(currentNode->data, curr);
        printf("ok %d\n", len - 2);
    }
}

/**
 * It removes the file or directory at the given path, both from the tree and from the HashMap.
 * It returns 0 as an error if the Node at the given path doesn't exists, or if it is a
 * directory with non empty content. It returns 1 otherwise
 */
int delete(char *curr) {
    char *temp;
    Node *parent;
    Node *son;

    son = find(curr);
    temp = strrchr(curr, '/');
    temp[0] = '\0';
    parent = find(curr);
    temp[0] = '/';

    if (son && !(son->number_of_sons)) {
        parent->number_of_sons--;
        if (parent->son == son) {
            parent->son = son->next;
        } else {
            parent = parent->son;
            while (parent->next != son)
                parent = parent->next;
            parent->next = son->next;
        }
        hash_delete(son);
        free(son->path);
        if (son->data)
            free(son->data);
        free(son);
        global_counter--;
        return 1;
    }
    return 0;
}

/**
 * It removes the given node from the global Hash Table
 */
void hash_delete(Node *old) {
    unsigned int pos, hole, temporary;

    pos = jenkins_one_at_a_time_hash(old->path, strlen(old->path));
    while (hash_table[pos] != old)
        pos = (pos + 1) % hash_table_size;
    hash_table[pos] = NULL;
    hole = pos;
    pos = (pos + 1) % hash_table_size;

    //it resets the position of the following nodes in the HashMap, in case the current node collided with other nodes
    while (hash_table[pos])
    {
        temporary = jenkins_one_at_a_time_hash(hash_table[pos]->path, strlen(hash_table[pos]->path));

        if (temporary <= hole) {
            hash_table[hole] = hash_table[pos];
            hash_table[pos] = NULL;
            hole = pos;
        }
        pos = (pos + 1) % hash_table_size;
    }
}

/**
 * It returns all resources with the given name, in lexicographic order,
 * by generating a LinkedList of all the nodes in the tree, in order to explore it
 */
void findAll(char *curr) {
    int list_allocated_size = REALLOC_STEP;
    int next_free_slot = 0;
    char *resource_name;
    char **list;         //dynamic array containing all the found paths
    Linked_node *head;   
    Linked_node *tail;
    Linked_node *temp;

    if (!(root->son))
        printf("no\n");
    else {
        head = (Linked_node *) malloc(sizeof(Linked_node));
        list = (char **) malloc(list_allocated_size * sizeof(char *));
        head->current = root->son;
        tail = head;
        head->next = NULL;
        list[0] = NULL;

        while (head) {
            resource_name = strrchr(head->current->path, '/');
            resource_name = &resource_name[1];
            if (!strcmp(resource_name, curr)) {
                if (next_free_slot == list_allocated_size) {
                    list = realloc(list, sizeof(char *) * (list_allocated_size += REALLOC_STEP));
                }
                list[next_free_slot] = head->current->path;
                next_free_slot++;
            }
            //it now adds to the tail of the list, the nodes head->next and head->son, if they are present
            if (head->current->next)
            {
                temp = (Linked_node *) malloc(sizeof(Linked_node));
                temp->current = head->current->next;
                temp->next = NULL;
                tail->next = temp;
                tail = temp;
            }
            if (head->current->son) {
                temp = (Linked_node *) malloc(sizeof(Linked_node));
                temp->current = head->current->son;
                temp->next = NULL;
                tail->next = temp;
                tail = temp;
            }
            temp = head;
            head = head->next;
            free(temp);
        }
        if (!next_free_slot)
            printf("no\n");
        else {
            list = realloc(list, sizeof(char *) * (next_free_slot));
            qsort(list, next_free_slot, sizeof(char *), cstring_cmp);
            for (int t = 0; t < next_free_slot; t++)
                printf("ok %s\n", list[t]);
        }
        free(list);
    }
}

/**
 * It removes the Node at the given path, if present, and also all of the subdirectories of the given path.
 * First, it adds all of the elements to remove to a list, and then remove them one by one
 */
void delete_r(char *curr) {
    Node *parent;
    Node **list; //dynamic array containing all the elements to remove
    int current_element = 0;
    int next_free_slot = 0;
    int stack_allocated_size = REALLOC_STEP;

    parent = find(curr);
    if (parent == NULL)
        printf("no\n");
    else if (!(parent->son)) {
        delete(curr);
        printf("ok\n");
    } else {
        list = (Node **) malloc(stack_allocated_size * sizeof(Node *));
        list[current_element] = parent->son;
        parent->number_of_sons = 0;
        while (current_element <= next_free_slot) {
            if (list[current_element]->son) {
                next_free_slot++;
                if (next_free_slot == stack_allocated_size)
                    list = realloc(list, sizeof(Node *) * (stack_allocated_size += REALLOC_STEP));
                list[next_free_slot] = list[current_element]->son;
            }
            if (list[current_element]->next) {
                next_free_slot++;
                if (next_free_slot == stack_allocated_size)
                    list = realloc(list, sizeof(Node *) * (stack_allocated_size += REALLOC_STEP));
                list[next_free_slot] = list[current_element]->next;
            }
            current_element++;
        }
        while (next_free_slot >= 0) {
            hash_delete(list[next_free_slot]);
            free(list[next_free_slot]->path);
            if (list[next_free_slot]->data)
                free(list[next_free_slot]->data);
            free(list[next_free_slot]);
            global_counter--;
            next_free_slot--;
        }
        delete(curr);
        printf("ok\n");
        free(list);
    }
}

/**
 * Hashing Function
 */
uint32_t jenkins_one_at_a_time_hash(char *key, size_t length) {
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return (hash % hash_table_size);
}

/**
 * String Compare
 */
int cstring_cmp(const void *a, const void *b) {
    const char **ia = (const char **) a;
    const char **ib = (const char **) b;
    return strcmp(*ia, *ib);
}

/**
 * Multiply by 4 the size of the hash map
 */
void increase_hashMap() {
    free(hash_table);
    hash_table_size = hash_table_size * 4;
    hash_table = (Node **) calloc(hash_table_size, sizeof(Node *));
    recompute_hashes(root->son);
}

/**
 * Recursive function used to recompute the hash of all the nodes
 */
void recompute_hashes(Node *el) {
    if (el->son)
        recompute_hashes(el->son);
    if (el->next)
        recompute_hashes(el->next);
    add_to_hashMap(el);
}
