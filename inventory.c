#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_ITEMS 1000
#define CODE_LEN  20
#define NAME_LEN  50
#define DATA_FILE "inventory.txt"

typedef struct {
    char code[CODE_LEN];
    char name[NAME_LEN];
    int quantity;
    double price;
    int reorderLevel;
} Item;

/* The inventory is stored in an array of structures. */
Item inventory[MAX_ITEMS];
int itemCount = 0;

/* ---------- Input helpers ---------- */

void removeNewline(char *text) {
    size_t length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
    }
}

void readLine(const char *prompt, char *buffer, size_t size) {
    for (;;) {
        printf("%s", prompt);
        if (fgets(buffer, (int)size, stdin) == NULL) {
            printf("\nInput ended. Exiting.\n");
            exit(0);
        }

        if (strchr(buffer, '\n') == NULL) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
                /* discard extra characters */
            }
        }
        removeNewline(buffer);

        if (buffer[0] != '\0') {
            return;
        }
        printf("Input cannot be empty. Please try again.\n");
    }
}

int readInt(const char *prompt, int minValue, int maxValue) {
    char buffer[100];
    char *end;
    long value;

    for (;;) {
        readLine(prompt, buffer, sizeof(buffer));
        errno = 0;
        value = strtol(buffer, &end, 10);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (errno == 0 && end != buffer && *end == '\0' &&
            value >= minValue && value <= maxValue) {
            return (int)value;
        }
        printf("Please enter a whole number from %d to %d.\n",
               minValue, maxValue);
    }
}

double readDouble(const char *prompt, double minValue) {
    char buffer[100];
    char *end;
    double value;

    for (;;) {
        readLine(prompt, buffer, sizeof(buffer));
        errno = 0;
        value = strtod(buffer, &end);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (errno == 0 && end != buffer && *end == '\0' &&
            value >= minValue) {
            return value;
        }
        printf("Please enter a valid number greater than or equal to %.2f.\n",
               minValue);
    }
}

/* Case-insensitive text comparison helpers (standard C, no POSIX functions). */
int compareIgnoreCase(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) {
            return ca - cb;
        }
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}  

int containsIgnoreCase(const char *text, const char *pattern) {
    size_t i, j;

    if (pattern[0] == '\0') {
        return 1;
    }

    for (i = 0; text[i] != '\0'; i++) {
        for (j = 0; pattern[j] != '\0'; j++) {
            if (text[i + j] == '\0' ||
                tolower((unsigned char)text[i + j]) !=
                tolower((unsigned char)pattern[j])) {
                break;
            }
        }
        if (pattern[j] == '\0') {
            return 1;
        }
    }
    return 0;
}

/* ---------- File handling ---------- */

/*
 * File format (one item per line):
 * code|name|quantity|price|reorderLevel
 *
 * Do not use the | character in an item code or name.
 */
void saveInventory(void) {
    FILE *file = fopen(DATA_FILE, "w");
    int i;

    if (file == NULL) {
        printf("Warning: Could not save data to %s.\n", DATA_FILE);
        return;
    }

    for (i = 0; i < itemCount; i++) {
        fprintf(file, "%s|%s|%d|%.2f|%d\n",
                inventory[i].code,
                inventory[i].name,
                inventory[i].quantity,
                inventory[i].price,
                inventory[i].reorderLevel);
    }

    fclose(file);
}

void loadInventory(void) {
    FILE *file = fopen(DATA_FILE, "r");
    char line[200];

    if (file == NULL) {
        /* First run: file will be created when data is saved. */
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL &&
           itemCount < MAX_ITEMS) {
        Item item;
        char *token;

        removeNewline(line);
        token = strtok(line, "|");
        if (token == NULL) continue;
        snprintf(item.code, sizeof(item.code), "%s", token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        snprintf(item.name, sizeof(item.name), "%s", token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        item.quantity = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        item.price = atof(token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        item.reorderLevel = atoi(token);

        if (item.quantity >= 0 && item.price >= 0.0 &&
            item.reorderLevel >= 0) {
            inventory[itemCount++] = item;
        }
    }

    fclose(file);
}

/* ---------- Inventory utility functions ---------- */

int findItemByCode(const char *code) {
    int i;
    /* Linear search: O(n). */
    for (i = 0; i < itemCount; i++) {
        if (compareIgnoreCase(inventory[i].code, code) == 0) {
            return i;
        }
    }
    return -1;
}

void printTableHeader(void) {
    printf("\n%-5s %-18s %-28s %10s %12s %12s\n",
           "No.", "Item Code", "Item Name", "Quantity",
           "Price", "Reorder At");
    printf("-------------------------------------------------------------------------------------------\n");
}

void printItem(int index, int serial) {
    printf("%-5d %-18s %-28s %10d %12.2f %12d\n",
           serial,
           inventory[index].code,
           inventory[index].name,
           inventory[index].quantity,
           inventory[index].price,
           inventory[index].reorderLevel);
}

void displayAllItems(void) {
    int i;

    if (itemCount == 0) {
        printf("\nInventory is empty. Add some items first.\n");
        return;
    }

    printTableHeader();
    for (i = 0; i < itemCount; i++) {
        printItem(i, i + 1);
    }
}

void addItem(void) {
    Item item;

    if (itemCount >= MAX_ITEMS) {
        printf("\nInventory is full. Maximum capacity is %d items.\n",
               MAX_ITEMS);
        return;
    }

    printf("\n--- Add New Item ---\n");
    readLine("Enter item code: ", item.code, sizeof(item.code));

    if (strchr(item.code, '|') != NULL) {
        printf("The | character is not allowed in item codes.\n");
        return;
    }

    if (findItemByCode(item.code) != -1) {
        printf("An item with this code already exists.\n");
        return;
    }

    readLine("Enter item name: ", item.name, sizeof(item.name));
    if (strchr(item.name, '|') != NULL) {
        printf("The | character is not allowed in item names.\n");
        return;
    }

    item.quantity = readInt("Enter opening quantity: ", 0, 1000000000);
    item.price = readDouble("Enter price per item: ", 0.0);
    item.reorderLevel = readInt("Enter low-stock/reorder level: ",
                                0, 1000000000);

    inventory[itemCount++] = item;
    saveInventory();
    printf("Item added successfully.\n");
}

void updateItem(void) {
    char code[CODE_LEN];
    int index;
    int choice;

    printf("\n--- Update Item ---\n");
    readLine("Enter item code to update: ", code, sizeof(code));
    index = findItemByCode(code);

    if (index == -1) {
        printf("Item not found.\n");
        return;
    }

    printf("1. Update name\n");
    printf("2. Update price\n");
    printf("3. Update reorder level\n");
    printf("4. Update all details (name, price, reorder level)\n");
    choice = readInt("Choose an option: ", 1, 4);

    if (choice == 1 || choice == 4) {
        readLine("Enter new item name: ",
                 inventory[index].name, sizeof(inventory[index].name));
        if (strchr(inventory[index].name, '|') != NULL) {
            printf("The | character is not allowed. Update cancelled.\n");
            return;
        }
    }
    if (choice == 2 || choice == 4) {
        inventory[index].price = readDouble("Enter new price: ", 0.0);
    }
    if (choice == 3 || choice == 4) {
        inventory[index].reorderLevel =
            readInt("Enter new reorder level: ", 0, 1000000000);
    }

    saveInventory();
    printf("Item updated successfully.\n");
}

void deleteItem(void) {
    char code[CODE_LEN];
    int index, i;
    char answer[10];

    printf("\n--- Remove Discontinued Item ---\n");
    readLine("Enter item code to remove: ", code, sizeof(code));
    index = findItemByCode(code);

    if (index == -1) {
        printf("Item not found.\n");
        return;
    }

    printf("Selected item: %s (%s)\n",
           inventory[index].name, inventory[index].code);
    readLine("Are you sure you want to delete it? (y/n): ",
             answer, sizeof(answer));

    if (tolower((unsigned char)answer[0]) != 'y') {
        printf("Delete cancelled.\n");
        return;
    }

    /* Shift later array elements one position to the left. */
    for (i = index; i < itemCount - 1; i++) {
        inventory[i] = inventory[i + 1];
    }
    itemCount--;

    saveInventory();
    printf("Item removed successfully.\n");
}

/* ---------- Stock In / Stock Out ---------- */

void stockIn(void) {
    char code[CODE_LEN];
    int index, amount;

    printf("\n--- Stock In (Purchase) ---\n");
    readLine("Enter item code: ", code, sizeof(code));
    index = findItemByCode(code);

    if (index == -1) {
        printf("Item not found. Add the item first.\n");
        return;
    }

    amount = readInt("Enter quantity to add: ", 1, 1000000000);
    if (inventory[index].quantity > 1000000000 - amount) {
        printf("Stock quantity would exceed the allowed limit.\n");
        return;
    }

    inventory[index].quantity += amount;
    saveInventory();
    printf("Stock increased. New quantity: %d\n",
           inventory[index].quantity);
}

void stockOut(void) {
    char code[CODE_LEN];
    int index, amount;

    printf("\n--- Stock Out (Sale) ---\n");
    readLine("Enter item code: ", code, sizeof(code));
    index = findItemByCode(code);

    if (index == -1) {
        printf("Item not found.\n");
        return;
    }

    amount = readInt("Enter quantity to remove/sell: ", 1, 1000000000);

    if (amount > inventory[index].quantity) {
        printf("Not enough stock. Available quantity: %d\n",
               inventory[index].quantity);
        return;
    }

    inventory[index].quantity -= amount;
    saveInventory();
    printf("Stock decreased. Remaining quantity: %d\n",
           inventory[index].quantity);
}

/* ---------- Search and low-stock alert ---------- */

void searchItems(void) {
    char query[NAME_LEN];
    int i, found = 0;

    printf("\n--- Search Items ---\n");
    readLine("Enter item code or part of item name: ",
             query, sizeof(query));

    for (i = 0; i < itemCount; i++) {
        if (compareIgnoreCase(inventory[i].code, query) == 0 ||
            containsIgnoreCase(inventory[i].name, query)) {
            if (!found) {
                printTableHeader();
            }
            printItem(i, i + 1);
            found = 1;
        }
    }

    if (!found) {
        printf("No matching item found.\n");
    }
}

void lowStockAlert(void) {
    int i, found = 0;

    printf("\n--- Low-Stock Items ---\n");
    for (i = 0; i < itemCount; i++) {
        if (inventory[i].quantity <= inventory[i].reorderLevel) {
            if (!found) {
                printTableHeader();
            }
            printItem(i, i + 1);
            found = 1;
        }
    }

    if (!found) {
        printf("No low-stock items. All items are above their reorder levels.\n");
    }
}

/* ---------- Sorting algorithms ---------- */

void swapItems(Item *a, Item *b) {
    Item temp = *a;
    *a = *b;
    *b = temp;
}

/* Bubble Sort: ascending price, O(n^2) average/worst case. */
void bubbleSortByPrice(void) {
    int i, j;

    for (i = 0; i < itemCount - 1; i++) {
        int swapped = 0;
        for (j = 0; j < itemCount - i - 1; j++) {
            if (inventory[j].price > inventory[j + 1].price) {
                swapItems(&inventory[j], &inventory[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

/* Selection Sort: ascending quantity, O(n^2). */
void selectionSortByQuantity(void) {
    int i, j, minIndex;

    for (i = 0; i < itemCount - 1; i++) {
        minIndex = i;
        for (j = i + 1; j < itemCount; j++) {
            if (inventory[j].quantity < inventory[minIndex].quantity) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            swapItems(&inventory[i], &inventory[minIndex]);
        }
    }
}

/* Insertion Sort: alphabetical item name, O(n^2) worst case. */
void insertionSortByName(void) {
    int i;
    for (i = 1; i < itemCount; i++) {
        Item key = inventory[i];
        int j = i - 1;

        while (j >= 0 &&
               compareIgnoreCase(inventory[j].name, key.name) > 0) {
            inventory[j + 1] = inventory[j];
            j--;
        }
        inventory[j + 1] = key;
    }
}

void sortingMenu(void) {
    int choice;

    if (itemCount == 0) {
        printf("\nNo items available to sort.\n");
        return;
    }

    printf("\n--- Sorting Menu ---\n");
    printf("1. Sort by price (Bubble Sort)\n");
    printf("2. Sort by quantity (Selection Sort)\n");
    printf("3. Sort by name (Insertion Sort)\n");
    choice = readInt("Choose sorting method: ", 1, 3);

    switch (choice) {
        case 1:
            bubbleSortByPrice();
            printf("Items sorted by price (low to high).\n");
            break;
        case 2:
            selectionSortByQuantity();
            printf("Items sorted by quantity (low to high).\n");
            break;
        case 3:
            insertionSortByName();
            printf("Items sorted alphabetically by name.\n");
            break;
    }

    displayAllItems();
    /* Sorting changes only the in-memory display order, not the file order. */
}

/* ---------- Inventory report ---------- */

void inventoryReport(void) {
    int i;
    long long totalUnits = 0;
    double totalValue = 0.0;

    printf("\n========== INVENTORY REPORT ==========\n");
    printf("Number of different items: %d\n", itemCount);

    for (i = 0; i < itemCount; i++) {
        totalUnits += inventory[i].quantity;
        totalValue += inventory[i].quantity * inventory[i].price;
    }

    printf("Total stock units: %lld\n", totalUnits);
    printf("Total inventory value: %.2f\n", totalValue);
    printf("======================================\n");

    displayAllItems();
}

/* ---------- Main menu ---------- */

void showMenu(void) {
    printf("\n============================================\n");
    printf("     INVENTORY AND STOCK MANAGEMENT SYSTEM\n");
    printf("============================================\n");
    printf("1.  Add new item\n");
    printf("2.  Update item details\n");
    printf("3.  Remove discontinued item\n");
    printf("4.  Stock In (purchase)\n");
    printf("5.  Stock Out (sale)\n");
    printf("6.  Search item\n");
    printf("7.  Display low-stock items\n");
    printf("8.  Display all items\n");
    printf("9.  Sort inventory\n");
    printf("10. Generate inventory report\n");
    printf("0.  Save and exit\n");
    printf("--------------------------------------------\n");
}

int main(void) {
    int choice;

    loadInventory();

    printf("Welcome to Inventory and Stock Management System!\n");
    printf("Loaded %d item(s) from %s.\n", itemCount, DATA_FILE);

    do {
        showMenu();
        choice = readInt("Enter your choice: ", 0, 10);

        switch (choice) {
            case 1:  addItem(); break;
            case 2:  updateItem(); break;
            case 3:  deleteItem(); break;
            case 4:  stockIn(); break;
            case 5:  stockOut(); break;
            case 6:  searchItems(); break;
            case 7:  lowStockAlert(); break;
            case 8:  displayAllItems(); break;
            case 9:  sortingMenu(); break;
            case 10: inventoryReport(); break;
            case 0:
                saveInventory();
                printf("Data saved. Thank you for using the system!\n");
                break;
        }
    } while (choice != 0);

    return 0;
}
