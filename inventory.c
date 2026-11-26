#include <stdio.h>
#include <string.h>

#define MAX 100

struct Item {
    int code, quantity, reorder;
    char name[30];
    float price;
};

struct Item items[MAX];
int count = 0;

void save() {
    FILE *fp = fopen("inventory.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d %s %d %.2f %d\n",
        items[i].code, items[i].name,
        items[i].quantity, items[i].price,
        items[i].reorder);
    }
    fclose(fp);
}

void load() {
    FILE *fp = fopen("inventory.txt", "r");
    if (fp == NULL) return;

    while (fscanf(fp, "%d %29s %d %f %d",
    &items[count].code, items[count].name,
    &items[count].quantity, &items[count].price,
    &items[count].reorder) == 5 && count < MAX) {
        count++;
    }
    fclose(fp);
}

void addItem() {
    printf("Enter code, name, quantity, price, reorder level: ");
    scanf("%d %29s %d %f %d",
    &items[count].code, items[count].name,
    &items[count].quantity, &items[count].price,
    &items[count].reorder);

    count++;
    save();
    printf("Item added successfully!\n");
}

void display() {
    printf("\nCode\tName\tQuantity\tPrice\n");
    for (int i = 0; i < count; i++) {
        printf("%d\t%s\t%d\t\t%.2f\n",
        items[i].code, items[i].name,
        items[i].quantity, items[i].price);
    }
}

void searchItem() {
    int code, found = 0;
    printf("Enter item code: ");
    scanf("%d", &code);

    for (int i = 0; i < count; i++) {
        if (items[i].code == code) {
            printf("Name: %s\nQuantity: %d\nPrice: %.2f\n",
            items[i].name, items[i].quantity,
            items[i].price);
            found = 1;
        }
    }
    if (!found) printf("Item not found!\n");
}


void updateItem() {
    int code;
    printf("Enter item code: ");
    scanf("%d", &code);

    for (int i = 0; i < count; i++) {
        if (items[i].code == code) {
            printf("Enter new quantity and price: ");
            scanf("%d %f", &items[i].quantity,
                  &items[i].price);
            save();
            printf("Updated successfully!\n");
            return;
        }
    }
    printf("Item not found!\n");
}

void deleteItem() {
    int code;
    printf("Enter item code: ");
    scanf("%d", &code);

    for (int i = 0; i < count; i++) {
        if (items[i].code == code) {
            for (int j = i; j < count - 1; j++)
                items[j] = items[j + 1];

            count--;
            save();
            printf("Item deleted!\n");
            return;
        }
    }
    printf("Item not found!\n");
}

void stockIn() {
    int code, qty;
    printf("Enter code and quantity: ");
    scanf("%d %d", &code, &qty);

    for (int i = 0; i < count; i++) {
        if (items[i].code == code) {
            items[i].quantity += qty;
            save();
            printf("Stock added!\n");
            return;
        }
    }
    printf("Item not found!\n");
}

void stockOut() {
    int code, qty;
    printf("Enter code and quantity: ");
    scanf("%d %d", &code, &qty);

    for (int i = 0; i < count; i++) {
        if (items[i].code == code) {
            if (qty <= items[i].quantity) {
                items[i].quantity -= qty;
                save();
                printf("Stock removed!\n");
            } else {
                printf("Insufficient stock!\n");
            }
            return;
        }
    }
    printf("Item not found!\n");
}

void lowStock() {
    for (int i = 0; i < count; i++) {
        if (items[i].quantity <= items[i].reorder)
            printf("%s is low in stock!\n", items[i].name);
    }
}

void sortItems() {
    struct Item temp;
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (items[j].price > items[j + 1].price) {
                temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
    save();
    display();
}

void report() {
    float total = 0;
    int units = 0;

    for (int i = 0; i < count; i++) {
        units += items[i].quantity;
        total += items[i].quantity * items[i].price;
    }

    printf("Total Items: %d\n", count);
    printf("Total Units: %d\n", units);
    printf("Total Value: %.2f\n", total);
}

int main() {
    int choice;
    load();

    do {
        printf("\n--- INVENTORY MANAGEMENT ---\n");
        printf("1. Add Item\n");
        printf("2. Display Items\n");
        printf("3. Search Item\n");
        printf("4. Update Item\n");
        printf("5. Delete Item\n");
        printf("6. Stock In\n");
        printf("7. Stock Out\n");
        printf("8. Low Stock Alert\n");
        printf("9. Sort by Price\n");
        printf("10. Inventory Report\n");
        printf("0. Exit\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addItem(); break;
            case 2: display(); break;
            case 3: searchItem(); break;
            case 4: updateItem(); break;
            case 5: deleteItem(); break;
            case 6: stockIn(); break;
            case 7: stockOut(); break;
            case 8: lowStock(); break;
            case 9: sortItems(); break;
            case 10: report(); break;
            case 0: save(); printf("Thank you!\n"); break;
            default: printf("Invalid choice!\n");
        }

    } while (choice != 0);

    return 0;
}
