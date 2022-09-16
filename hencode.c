
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>

#define TABLESIZE 256
#define SIZE 20
#define COUNT 10
#define ZERO 0
#define PERMS 0666


struct node *char_table[TABLESIZE];
struct node *char_table2[TABLESIZE];
int read_char(FILE *file);
int the_count;
int cmp_node(const void *left, const void *right);
int insert(char *buf);
void print_array();
void print_array2();
void final_print();
void print_tree(struct node *root);
struct node *build_linked_list(struct node **nl);
void set_hcode(struct node *root, char *hcode, char c);
void print_hcodes(struct node *root, char *array, int i);
void add_paths(struct node *root);
void print2DUtil(struct node *root, int space);
struct node *build_tree(struct node *root);
void set_b(char *output, int bit_index, char b);

struct node {
        int ch;
        int count;
        struct node *next;
        struct node *left;
        struct node *right;
        char *path;
};


int main(int argc, char *argv[]) {

        int infile;
        int outfile;
        infile = STDIN_FILENO;
        outfile = STDOUT_FILENO;

        uint8_t uniq_count;
        uniq_count = 0;
        uint8_t buf;
        char ch_buf[BUFSIZ+1] = {0,};

        if (argv[1] != NULL) {
                infile = open(argv[1], O_RDONLY, PERMS);
        }
        if (infile == -1) {
                        perror("infile error");
                        exit(EXIT_FAILURE);
                }



        while (read(infile, ch_buf, 1) > 0) {
                if(insert(ch_buf) == 1)
                        uniq_count++;
        }
        uint8_t num;
        num = uniq_count - 1;


        /* saves a copy of histogram for later before qsorting */
        int i;
        for (i = 0; i < TABLESIZE; i++) {
                char_table2[i] = char_table[i];
        }

        qsort(char_table, TABLESIZE, sizeof(struct node*), cmp_node);

        struct node *linked_list;
        linked_list = build_linked_list(char_table);

        struct node *root = linked_list;
        if (root == NULL)
                return 0;

        root = build_tree(root);



        char *array = calloc(SIZE, sizeof(char));
        int total_size;
        total_size = 0;
        print_hcodes(root, array, 0);

        /* if output file is given */
        if (argv[2] != NULL) {
                outfile = creat(argv[2], PERMS);
                if (outfile < 0) {
                        perror("outfile error");
                        return -1;
                }
                /* write the number of unique characters */
                else if (outfile != -1) {
                        if (write(outfile, &num, sizeof(uint8_t)) == -1) {
                                perror("write error");
                                return -1;
                        }
                        int i;
                        int j;
                        /* write the encode header information */
                        for (i = 0; i < TABLESIZE; i++) {
                                if (char_table2[i] != NULL) {
                                        uint8_t ch = char_table2[i]->ch;
                                        write(outfile, &ch, sizeof(uint8_t));
                                        uint32_t count;
                                        count = char_table2[i]->count;
                                        count = htonl(count);
                                        write(outfile, &count,
                                                sizeof(uint32_t));
                                        total_size += (char_table2[i]->count *
                                        strlen(char_table2[i]->path));
                                }
                        }
                        fflush(stdout);
                        int total_size2;
                        total_size2 = 0;
                        if (total_size > 0)
                                total_size2 = (total_size-1)/8+1;
                        else
                                total_size2 = 0;
                        char *bits_arr;
                        char *temp_arr;
                        unsigned int bit_index;
                        bit_index = 0;
                        bits_arr = malloc(total_size2);
                        int infile2;
                        infile = open(argv[1], O_RDONLY, PERMS);
                        /*if (infile == -1) {
                                perror("infile error");
                                exit(EXIT_FAILURE);
                        }*/
                        /* write the encode body information */
                        int num1;
                        num1 = 0;
                        while (read(infile, ch_buf, 1) > 0) {
                                int ch;
                                ch = *ch_buf;
                                temp_arr = char_table2[ch] -> path;
                                for (i = 0; i < strlen(temp_arr);
                                        i++, bit_index++) {
                                        if (temp_arr[i] == '0') {
                                                set_b(bits_arr, bit_index, 0);
                                        }
                                        else {
                                                set_b(bits_arr, bit_index, 1);
                                        }
                                }
                                if (num++ > total_size2+uniq_count)
                                        close(infile);
                        }
                        write(outfile, bits_arr, total_size2);
                }
        }
        else {
                final_print();
        }

        close(infile);
        return 0;
}

void set_b(char *output, int bit_index, char b) {
        int big_index;
        big_index = bit_index/8;
        int small_index;
        small_index = bit_index%8;
        char mask;
        if (b == 1) {
                mask = (1 << (7 - small_index));
                output[big_index] = output[big_index] | mask;
        }
        else {
                mask = (1 << (7 - small_index)) ^ 0xff;
                output[big_index] = output[big_index] & mask;
        }
}

/* builds tree from linked list*/
struct node *build_tree(struct node *root) {
        struct node *node1;
        struct node *node2;
        struct node *ptr;

        while(root->next != NULL) {
                node1 = root;
                root = root->next;
                node2 = root;
                root = (struct node *) malloc(sizeof(*root));

                root->left = node1;
                root->right = node2;
                root->count = (root->left->count + root->right->count);
                root->next = node2->next;
                ptr = node2->next;
                if (ptr != NULL && root->count > ptr->count) {
                        /* moves ptr along linked list to one 
 *                         position previous
 *                         where root should be placed to be in order */
                        while(ptr->next != NULL &&
                                        root->count > ptr->next->count) {
                                ptr=ptr->next;
                        }
                        root->next = ptr->next;
                        ptr->next = root;
                        /* resets the root to the old root's next */
                        root = node2->next;
                }

        }
        return root;

}



/* tranverses depth first through tree and assigns 
 * each leaf a huffman path */
void print_hcodes(struct node *root, char *array, int i) {
        if (root->left) {
                array[i] = '0';
                print_hcodes(root->left, array, i + 1);
        }
        if (root->right) {
                array[i] = '1';
                print_hcodes(root->right, array, i + 1);
        }
        if (!root->left && root->right)
                printf("root->left == NULL %d %c", root->ch, root->ch);

        if (!root->right && root->left)
                printf("root->right == NULL %d %c", root->ch, root->ch);

        if (!root->right && !root->left) {
                root->path = (char*) calloc(100 ,sizeof(char));
                if (strlen(array) > i)
                        array[i] = '\0';
                strcpy(root->path, array);
                /* updates nodes in original histogram with huffman paths */

                char_table2[root->ch]->path = root->path;
        }
}


struct node *build_linked_list(struct node **nl) {
        struct node *list;
        struct node* newNode;
        newNode = (struct node *) malloc(sizeof(*newNode));
        if (newNode == NULL)
            perror("null malloc");

        if (nl[0] != NULL) {
            newNode -> ch = nl[0] -> ch;
            newNode -> count = nl[0] -> count;
            newNode->next = NULL;
            newNode->left = NULL;
            newNode->right = NULL;
        }
        list = newNode;

        int i;
        /* loop through histogram and move any non-null */
        /* items into a linked list */
        for (i = 1; i < TABLESIZE; i++) {
                if (nl[i] != NULL) {
                        struct node* newNode =
                                (struct node *) malloc(sizeof(*newNode));
                        if (newNode == NULL)
                                perror("null malloc");
                        newNode -> ch = nl[i] -> ch;
                        newNode -> count = nl[i] -> count;
                        newNode -> next = list;
                        list = newNode;
                        newNode->left = NULL;
                        newNode->right = NULL;

                }
        }
        return list;
}


/*this compares two nodes in a way that allows qsort to sort*/
/*the histogram in ascending count order and secondarily*/
/*in ascending alphabetical order*/

int cmp_node(const void *left, const void *right) {

        const struct node *ln = *(const struct node **)left;
        const struct node *rn = *(const struct node **)right;

        if (!ln && !rn)
                return 0;
        if (!ln)
                return 1;
        if (!rn)
                return -1;
        if ((ln->count - rn->count) == 0)
                return (rn->ch - ln->ch);

        return (rn->count - ln->count);
}


/*inserts characters from file into histogram to get accurate count*/
int insert(char *buf)
{
        int k;
        k = *buf;
        struct node *n;
        if (char_table[k] == NULL) {
                n = malloc(sizeof(*n));
                n->count = 1;
                n->ch = k;
                char_table[k] = n;
                the_count++;
                n->left = NULL;
                n->right = NULL;
                n->next = NULL;
                n->path = NULL;
                return 1;
        }
        else
                char_table[k]->count++;
        return 0;

}

void final_print() {
        int i;
        for (i = 0; i < TABLESIZE; i++) {
                if (char_table2[i] != NULL) {
                        printf("0x%02x: %s\n", i, char_table2[i]->path);
                }
        }
}

void print_array() {
        struct node *n;
        int i;
        for (i = 0; i < TABLESIZE; i++) {
                n = char_table[i];
                if (n != NULL) {
                        printf("index/ascii: %d", i);
                        printf(" char: %c", n->ch);
                        printf(" count: %d", n->count);
                        printf(" path: %s\n", n->path);
                }
        }
        printf("number of chars: %d\n", the_count);

}

void print_array2() {
        struct node *n;
        int i;
        for (i = 0; i < TABLESIZE; i++) {
                n = char_table2[i];
                if (n != NULL) {
                        printf("index/ascii: %d", i);
                        printf(" char: %c", n->ch);
                        printf(" count: %d", n->count);
                        printf(" path: %s\n", n->path);
                }
        }
        printf("number of chars: %d\n", the_count);

}


void print_tree(struct node *root) {
        if (root!=NULL)
        {
                print_tree(root->left);
                if (root->left==NULL && root->right== NULL) {
                printf("ascii: %d", root->ch);
                printf(" char: %c", root->ch);
                printf(" count: %d", root->count);
                printf(" path: %s\n", root->path);
                }
                else
                {
                        printf("ascii: %d", root->ch);
                printf(" char: %c", root->ch);
                printf(" count: %d", root->count);
                printf(" left: %d", root->left->count);
                printf(" right: %d\n", root->right->count);

                }
                print_tree(root->right);
        }

}

void print2DUtil(struct node *root, int space)
{
         if (root == NULL)
                 return;

        space += COUNT;

        print2DUtil(root->right, space);

                 printf("\n");
         int i;
         for (i = COUNT; i < space; i++)
                   printf(" ");
         printf("%c %d\n", root->ch, root->count);

                print2DUtil(root->left, space);

}
