typedef int ElementType;

typedef struct AVLNode *Position;
typedef struct AVLNode *AVLTree;

struct AVLNode {
    ElementType Element;
    AVLTree Left; 
    AVLTree Right;
    int Height;
};

AVLTree AVL_Insert( ElementType X, AVLTree T );
AVLTree AVL_Erase( ElementType X, AVLTree T );
AVLTree AVL_Find( ElementType X, AVLTree T );
void printInorder(AVLTree T);

void DeleteTree(AVLTree T);
