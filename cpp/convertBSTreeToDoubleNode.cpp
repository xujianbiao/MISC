#include <iostream>
#include <random>
using namespace std;

struct BSTreeNode {
    int val;
    BSTreeNode *lchild, *rchild;
};

typedef BSTreeNode DoubleList;
DoubleList *pHead, *pLastIndex;

void createBSTree(BSTreeNode* &pCurrentNode, int val) {
    // 利用中序遍历创建二叉树
    if (!pCurrentNode) {
        BSTreeNode *tmp = new BSTreeNode();
        tmp->val = val;
        tmp->lchild = NULL;
        tmp->rchild = NULL;
        pCurrentNode = tmp;
    }
    else {
        if (pCurrentNode->val > val)
            createBSTree(pCurrentNode->lchild, val);
        else
            createBSTree(pCurrentNode->rchild, val);
    }
}

void convertBSTreeToDoubleList(BSTreeNode *node) {
    if (!node)
        return;

    if (node->lchild)
        convertBSTreeToDoubleList(node->lchild);

    node->lchild = pLastIndex;
    if (!pLastIndex)
        pHead = node;
    else
        pLastIndex->rchild = node;
    pLastIndex = node;

    if (node->rchild)
        convertBSTreeToDoubleList(node->rchild);

}

int main(int argc, char *argv[]) {
    BSTreeNode *pRoot;
    pRoot = NULL;
    pHead = NULL;
    pLastIndex = NULL;

    createBSTree(pRoot,4);
    createBSTree(pRoot,10);
    createBSTree(pRoot,6);
    createBSTree(pRoot,16);
    createBSTree(pRoot,14);
    createBSTree(pRoot,12);
    createBSTree(pRoot,8);

    convertBSTreeToDoubleList(pRoot);
    while (pHead) {
        cout << pHead->val << endl;
        pHead = pHead->rchild;
    }
    return 0;
}