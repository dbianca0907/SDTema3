#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."


FileTree createFileTree(char* rootFolderName) {
    FileTree file;
    file.root = calloc(1, sizeof(TreeNode));

    //  egalam direct numele root-ului pentru a elibera si
    //  memoria de la strdup("root")
    file.root->name = rootFolderName;
    file.root->type = FOLDER_NODE;

    return file;
}

TreeNode* get_through_dir(TreeNode *currentNode, char *fileName) {
    //printf("%p %s\n", currentNode, fileName);
    if (currentNode->content == NULL)
        return NULL;

    List* list = ((FolderContent*)currentNode->content)->children;
    ListNode *curr = list->head;
    //printf("%p\n", list->head);
    //printf("head = %s\n", list->head->info->name);
    while (curr != NULL) {
        //printf("%s\n", curr->info->name);
        if (strcmp(curr->info->name, fileName) == 0)
            return curr->info;
        curr = curr->next;
    }
    return NULL;
}

void create_node(ListNode **node, TreeNode* currentNode,
                 char* fileName, char* fileContent, enum TreeNodeType type) {
    (*node)->info = calloc(1, sizeof(TreeNode));
    (*node)->info->parent = calloc(1, sizeof(TreeNode));
    memcpy((*node)->info->parent, currentNode, sizeof(TreeNode));
    (*node)->info->name = malloc(strlen(fileName) + 1);
    memcpy((*node)->info->name, fileName, strlen(fileName) + 1);
    (*node)->info->type = type;

    if (type == FILE_NODE) {
        (*node)->info->content = calloc(1, sizeof(FileContent));
        FileContent *content = (*node)->info->content;
        content->text = malloc(strlen(fileContent) + 1);
        memcpy(((FileContent*)(*node)->info->content)->text,
               fileContent, strlen(fileContent) + 1);
    }
}

void add_node(List **list, TreeNode* currentNode, char* fileName,
               char* fileContent, enum TreeNodeType type) {
    if (*list == NULL) {
        *list = calloc(1, sizeof(List));
    }
    if ((*list)->head == NULL) {
        (*list)->head = calloc(1, sizeof(ListNode));
        create_node(&((*list)->head), currentNode,
                    fileName, fileContent, type);
    } else {
        ListNode *new = calloc(1, sizeof(ListNode));
        create_node(&new, currentNode, fileName, fileContent, type);
        new->next = (*list)->head;
        (*list)->head = new;
    }
}

void ls(TreeNode* currentNode, char* arg) {
    if (strcmp(arg, "\0") == 0) {
        List *list = ((FolderContent*)currentNode->content)->children;
        ListNode *curr = list->head;
        while (curr != NULL) {
            printf("%s\n", curr->info->name);
            curr = curr->next;
        }
        return;
    }

    TreeNode *node = get_through_dir(currentNode, arg);
    if (node == NULL) {
        printf("ls: cannot access '%s': No such file or directory\n", arg);
        return;
    }

    if (node->type == FILE_NODE) {
        printf("%s: %s\n", arg, ((FileContent*)node->content)->text);
    } else {
        if (node->content == NULL) {
            printf("\n");
            return;
        }
        List *list = ((FolderContent*)node->content)->children;
        ListNode *curr = list->head;
        while (curr != NULL) {
            printf("%s\n", curr->info->name);
            curr = curr->next;
        }
    }
}


void pwd(TreeNode* treeNode) {
}


TreeNode* cd(TreeNode* currentNode, char* path) {
    char *aux_path = malloc(strlen(path) + 1);
    memcpy(aux_path, path, strlen(path) + 1);

    char *pointer = strtok(aux_path, "/\n\0");
    while (pointer != NULL) {
        TreeNode *node = get_through_dir(currentNode, pointer);
        if (node == NULL && strcmp(pointer, "..")) {
            printf("cd: no such file or directory: %s", path);
            free(aux_path);
            return currentNode;
        } else {
            if (strcmp(pointer, "..") == 0) {
                currentNode = currentNode->parent;
            } else {
                currentNode = node;
            }
        }
        pointer = strtok(NULL, "/\n\0");
    }
    return currentNode;
}


void tree(TreeNode* currentNode, char* arg) {
}


void mkdir(TreeNode* currentNode, char* folderName) {
    if (get_through_dir(currentNode, folderName) != NULL) {
        printf("mkdir: cannot create directory ‘%s’: File exists\n",
               folderName);
        return;
    }

    if (currentNode->content == NULL) {
        currentNode->content = calloc(1, sizeof(FolderContent));
        FolderContent* content = currentNode->content;
        content->children = calloc(1, sizeof(List));
    }
    List *list = ((FolderContent*)currentNode->content)->children;
    add_node(&list, currentNode, folderName, NULL, FOLDER_NODE);

    // free(folderName);
}

void remove_directory(TreeNode *currentNode) {
    
    if (currentNode->content == NULL) {
        return;
    }

    List *list = ((FolderContent*) currentNode->content)->children;
    ListNode *curr = list->head;
    
    while (list->head != NULL) {
        curr = list->head;
        list->head = list->head->next;

        if (curr->info->type == FILE_NODE) {
            free(curr->info->parent);
            free(curr->info->name);
            if (curr->info->type == FILE_NODE) {
                free(((FileContent*)curr->info->content)->text);
                free(curr->info->content);
            }
            free(curr->info);
            free(curr);        
        } else {
            remove_directory(curr->info);
            free(currentNode->parent);
            free(currentNode->name);
            free(currentNode->content);
            free(currentNode);
        }
    }
    free(list);
}

void rmrec(TreeNode* currentNode, char* resourceName) {
    TreeNode *node = get_through_dir(currentNode, resourceName);
    if (node == NULL) {
        printf("rmrec: failed to remove '%s': No such file or directory\n",
               resourceName);
        return;
    }

    List *list = ((FolderContent*) currentNode->content)->children;
    ListNode *curr = list->head;
    ListNode *prev = NULL;

    if (strcmp(((TreeNode*) curr->info)->name, node->name) == 0) {
        list->head = list->head->next;
        remove_directory(node);
        free(curr->info->parent);
        free(curr->info->name);
        free(curr->info);
        free(curr);
        return;
    }

    while (curr != NULL) {
        if (strcmp(((TreeNode*) curr->info)->name, node->name) == 0) {
            prev->next = curr->next;
            remove_directory(node);
            free(curr->info->parent);
            free(curr->info->name);
            free(curr->info);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}


void rm(TreeNode* currentNode, char* fileName) {
    TreeNode *node = get_through_dir(currentNode, fileName);

    if (node == NULL) {
        printf("rm: failed to remove '%s': No such file or directory\n",
               fileName);
        return;
    }

    if (node->type == FOLDER_NODE) {
        printf("rm: cannot remove '%s': Is a directory\n", fileName);
        return;
    }

    List *list = ((FolderContent*) currentNode->content)->children;
    ListNode *curr = list->head;
    ListNode *prev = NULL;

    if (strcmp(((TreeNode*) curr->info)->name, node->name) == 0) {
        list->head = list->head->next;
        free(curr->info->parent);
        free(curr->info->name);
        free(((FileContent*)curr->info->content)->text);
        free(curr->info->content);
        free(curr->info);
        free(curr);
        return;
    }

    while (curr != NULL) {
        if (strcmp(((TreeNode*) curr->info)->name, node->name) == 0) {
            prev->next = curr->next;
            free(curr->info->parent);
            free(curr->info->name);
            free(((FileContent*)curr->info->content)->text);
            free(curr->info->content);
            free(curr->info);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}


void rmdir(TreeNode* currentNode, char* folderName) {
    TreeNode *node = get_through_dir(currentNode, folderName);

    if (node == NULL) {
        printf("rmdir: failed to remove '%s': No such file or directory\n",
               folderName);
        return;
    }

    if (node->type == FILE_NODE) {
        printf("rmdir: failed to remove '%s': Not a directory\n", folderName);
        return;
    }

    if (node->content != NULL) {
        printf("rmdir: failed to remove '%s': Directory not empty\n",
               folderName);
        return;
    }

    List *list = ((FolderContent*) currentNode->content)->children;
    ListNode *curr = list->head;
    ListNode *prev = NULL;

    if (strcmp(((TreeNode*) curr->info)->name, node->name) == 0) {
        list->head = list->head->next;
        free(curr->info->parent);
        free(curr->info->name);
        free(curr->info);
        free(curr);
        return;
    }

    while (curr != NULL) {
        if (strcmp(((TreeNode*) curr->info)->name, node->name) == 0) {
            prev->next = curr->next;
            free(curr->info->parent);
            free(curr->info->name);
            free(curr->info);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
    if (get_through_dir(currentNode, fileName) == NULL) {
        if (currentNode->content == NULL) {
            currentNode->content = calloc(1, sizeof(FolderContent));
            FolderContent* content = currentNode->content;
            content->children = calloc(1, sizeof(List));
        }
        List *list = ((FolderContent*)currentNode->content)->children;
        add_node(&list, currentNode, fileName, fileContent, FILE_NODE);
    }

    free(fileName);
    free(fileContent);
}


void cp(TreeNode* currentNode, char* source, char* destination) {
}

void mv(TreeNode* currentNode, char* source, char* destination) {
}

void freeTree(FileTree fileTree) {
    TreeNode* currentFolder = fileTree.root;
    if (currentFolder->content != NULL) {
        List *list = ((FolderContent*) currentFolder->content)->children;
        ListNode *curr = list->head;
        ListNode *tmp;
        while (curr != NULL) {
            tmp = curr->next;
            if (curr->info->type == FILE_NODE) {
                rm(currentFolder, curr->info->name);
            }
            curr = tmp;
            list->head = tmp;
        }
        free(list);
        free(currentFolder->content);
    }
    free(currentFolder->name);
    free(currentFolder);
}
