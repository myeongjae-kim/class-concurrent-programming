#include <stdio.h>
#include <stdlib.h>
#include "AVL.h"

#define MEM 1
#define FIL 2
#define ARG 1

void EIN(void* adr, int messageselector); // Error If Null
// messageselector: 0 or any number -> default, 1 or MEM -> memory, 2or FIL -> file.

void EIF(int integer, int messageselector); // Error If False
// messageselector: 0 or any number -> default, 1 or ARG-> argument.

void freeN(void** adr); // free() and insert NULL value to the pointer.
// The argument should be address of pointer variable.

typedef int ElementType;

typedef struct AVLNode *Position;
typedef struct AVLNode *AVLTree;

Position SingleRotateWithLeft( Position node );
Position SingleRotateWithRight( Position node );
Position DoubleRotateWithLeft( Position node );
Position DoubleRotateWithRight( Position node );
AVLTree AVL_Insert( ElementType X, AVLTree T );
AVLTree AVL_Erase( ElementType X, AVLTree T );
AVLTree AVL_Find( ElementType X, AVLTree T );
void printInorder(AVLTree T);
int getHeight(Position node);
int Max(int, int);

void DeleteTree(AVLTree T);

/* int main(int argc, char const *argv[])
 * {
 *     FILE *fp = NULL;
 *     ElementType buff = 0;
 *     AVLTree T = NULL;
 *
 *   EIF(argc == 4, ARG);
 *     EIN(fp = fopen(argv[1], "r"), MEM);
 *
 *     while(!feof(fp))
 *     {
 *         fscanf(fp, "%d", &buff);
 *         T = AVL_Insert(buff, T);
 *         printf("Height of root : %d\n", getHeight(T));
 *         printInorder(T);
 *         putchar('\n');
 *     }
 *
 *     EIN(fp = fopen(argv[2], "r"), MEM);
 *
 *     while(!feof(fp))
 *     {
 *         fscanf(fp, "%d", &buff);
 *         AVLTree temp_find = AVL_Find(buff, T);
 *
 *         printf("Find %d : %ld\n",buff, (long)temp_find);
 *
 *         // printInorder(T);
 *         // putchar('\n');
 *     }
 *
 *
 *     EIN(fp = fopen(argv[3], "r"), MEM);
 *
 *     while(!feof(fp))
 *     {
 *         fscanf(fp, "%d", &buff);
 *         T = AVL_Erase(buff, T);
 *         printf("Height of root : %d\n", getHeight(T));
 *         printInorder(T);
 *         putchar('\n');
 *     }
 *
 *
 *     fclose(fp);
 *     DeleteTree(T);
 *
 *     return 0;
 * } */
void EIN(void* adr, int messageselector) // Error If Null
{       // messageselector: 0 or any number -> default, 1 -> memory, 2 -> file.
    if(!adr)
    {
	switch (messageselector) {
	    case 1:
		fprintf(stderr, "Memory cannot be allocated!\n");
		break;
		
	    case 2:
		fprintf(stderr, "File Opening Error!\n");
		break;
		
	    default:
		fprintf(stderr, "Null Address Error!\n");
		break;
	}
	exit(EXIT_FAILURE);
    }
}

void EIF(int integer, int messageselector) // Error If FALSE
{       // messageselector: 0 or any number -> default, 1-> argument.
    if(!integer)
    {
	switch (messageselector) {
	    case 1:
		fprintf(stderr, "Command line argument error!\n");
		fprintf(stderr, "Usage: [executive file] [text file]\n");
		break;
		
	    default:
		fprintf(stderr, "FALSE! FALSE!\n");
		break;
	}
	exit(EXIT_FAILURE);
    }
}

void freeN(void** adr) {
    if(!(*adr)) {
	fprintf(stderr, "freeN(): The variable has NULL value. It cannot be free\n");
	return;
    }
    free(*adr);
    *adr = NULL;
}
Position SingleRotateWithLeft( Position K2 )
{
    Position K1;

    K1 = K2 -> Left; 

    K2 -> Left = K1 -> Right;
    K1 -> Right = K2;

    K2 -> Height = Max(getHeight(K2 -> Left),
        getHeight(K2 -> Right)) + 1;

    K1 -> Height = Max(getHeight(K1 -> Left), K2 -> Height) + 1;

    return K1; // New root

}
Position SingleRotateWithRight( Position K2 )
{
    Position K3;

    K3 = K2 -> Right; // Copy to empty node.

    K2 -> Right = K3 -> Left;
    K3 -> Left = K2;

    K2 -> Height = Max(getHeight(K2 -> Left),
        getHeight(K2 -> Right)) + 1;

    K3 -> Height = Max(K2 -> Height, getHeight(K3 -> Right)) + 1;

    return K3; // New root
}
Position DoubleRotateWithLeft( Position K3 )
{
    K3 -> Left = SingleRotateWithRight(K3 -> Left);
    return SingleRotateWithLeft(K3);
}
Position DoubleRotateWithRight( Position K3 )
{
    K3 -> Right = SingleRotateWithLeft(K3 -> Right);
    return SingleRotateWithRight(K3);
}
AVLTree AVL_Insert( ElementType X, AVLTree T )
{
    if(!T) {
        EIN(T = (AVLTree)malloc(sizeof(struct AVLNode)), MEM);
        T -> Element = X; T-> Height = 0;
        T -> Left = T -> Right = NULL;
    }
    else if(X < T -> Element) // Left , Right
    {
        T -> Left = AVL_Insert(X, T -> Left);
        if(getHeight(T -> Left) - getHeight(T -> Right) == 2)
        {
            if(X < T -> Left -> Element) // Single or double?
                T = SingleRotateWithLeft(T);
            else
                T = DoubleRotateWithLeft(T);
        }
    }
    else if(X > T -> Element)
    {
        T -> Right = AVL_Insert(X, T -> Right);
        if(getHeight(T -> Right) - getHeight(T -> Left) == 2)
        {
            if(X > T -> Right -> Element)
                T = SingleRotateWithRight(T);
            else
                T = DoubleRotateWithRight(T);
        }
    }

    //Height adjustment when inserted without rotation
    T -> Height = Max(getHeight(T -> Left), getHeight(T -> Right)) + 1;

    return T;
}
void printInorder(AVLTree T)
{
    if(T)
	{
    printInorder(T -> Left);
    printf("%d ", T -> Element); // base case.
    printInorder(T -> Right);
	}
}
int getHeight(Position node)
{
    if(!node)
        return -1;
    else
        return node -> Height;
}
int Max(int a, int b)
{
    return a > b ? a : b;
}

void DeleteTree(AVLTree T)
{
  if (T) {
    if(T -> Left)
        DeleteTree(T -> Left);
    if(T -> Right)
        DeleteTree(T -> Right);
	freeN((void**)&T); 
  }
}





// deletion from: http://www.geeksforgeeks.org/avl-tree-set-2-deletion/
// Recursive function to delete a node with given key
// from subtree with given root. It returns root of
// the modified subtree.

/* Given a non-empty binary search tree, return the
   node with minimum key value found in that tree.
   Note that the entire tree does not need to be
   searched. */
AVLTree minValueNode(AVLTree node)
{
    AVLTree current = node;
 
    /* loop down to find the leftmost leaf */
    while (current->Left != NULL)
        current = current->Left;
 
    return current;
}

// Get Balance factor of node N
int getBalance(AVLTree N)
{
    if (N == NULL)
        return 0;
    return getHeight(N->Left) - getHeight(N->Right);
}

// struct Node* deleteNode(struct Node* root, int key)
AVLTree AVL_Erase( ElementType X, AVLTree T )
{
    // STEP 1: PERFORM STANDARD BST DELETE
 
    if (T == NULL)
        return T;
 
    // If the key to be deleted is smaller than the
    // root's key, then it lies in left subtree
    if ( X < T->Element )
        T->Left = AVL_Erase(X, T->Left);
 
    // If the key to be deleted is greater than the
    // root's key, then it lies in right subtree
    else if( X > T->Element )
        T->Right = AVL_Erase(X, T->Right);
 
    // if key is same as root's key, then This is
    // the node to be deleted
    else
    {
        // node with only one child or no child
        if( (T->Left == NULL) || (T->Right == NULL) )
        {
            AVLTree temp = T->Left ? T->Left : T->Right;
 
            // No child case
            if (temp == NULL)
            {
                temp = T;
                T = NULL;
            }
            else // One child case
             *T = *temp; // Copy the contents of
                            // the non-empty child
            free(temp);
        }
        else
        {
            // node with two children: Get the inorder
            // successor (smallest in the right subtree)
            AVLTree temp = minValueNode(T->Right);
 
            // Copy the inorder successor's data to this node
            T->Element = temp->Element;
 
            // Delete the inorder successor
            T->Right = AVL_Erase(T->Element, T->Right);
        }
    }
 
    // If the tree had only one node then return
    if (T == NULL)
      return T;
 
    // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
    T->Height = 1 + Max(getHeight(T->Left),
                           getHeight(T->Right));
 
    // STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to
    // check whether this node became unbalanced)
    int balance = getBalance(T);
 
    // If this node becomes unbalanced, then there are 4 cases
 
    // Left Left Case
    if (balance > 1 && getBalance(T->Left) >= 0)
        return SingleRotateWithLeft(T);
 
    // Left Right Case
    if (balance > 1 && getBalance(T->Left) < 0)
    {
        return DoubleRotateWithLeft(T);
    }
 
    // Right Right Case
    if (balance < -1 && getBalance(T->Right) <= 0)
        return SingleRotateWithRight(T);
 
    // Right Left Case
    if (balance < -1 && getBalance(T->Right) > 0)
    {
        return DoubleRotateWithRight(T);
    }
 
    return T;
}

AVLTree AVL_Find( ElementType X, AVLTree T ){
    if(!T) {
      return nullptr;
    }
    else if(X < T -> Element) // Left , Right
    {
        return AVL_Find(X, T -> Left);
    }
    else if(X > T -> Element)
    {
        return AVL_Find(X, T -> Right);
    }
    else if (X == T -> Element) {
      return T;
    }

    return nullptr;
}
