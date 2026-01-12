#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP_MS(x) sleep(x / 1000)
#endif

#define MAX 10000  // Maximum tickets in RAM

struct Ticket {
    int ticketID;
    char customerName[50];
    char email[50];
    char product[50];
    char dateOfPurchase[30];
    char issueType[100];
    char status[30];
    char priority[20];
    char responseTime[30];
    char resolveTime[30];
};

struct Ticket queue[MAX];
int front = -1, rear = -1;

int isFull() { return (front == (rear + 1) % MAX); }
int isEmpty() { return (front == -1); }

void removeNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') str[len - 1] = '\0';
}

void getCurrentTime(char *buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void getAutoPriority(char *issue, char *priorityStr) {
    if (strstr(issue, "Hack") || strstr(issue, "Money") || strstr(issue, "Billing") || 
        strstr(issue, "Crash") || strstr(issue, "Security") || strstr(issue, "Data Loss")) {
        strcpy(priorityStr, "Critical");
    } else if (strstr(issue, "Error") || strstr(issue, "Fail") || strstr(issue, "Not working") || strstr(issue, "Urgent")) {
        strcpy(priorityStr, "High");
    } else if (strstr(issue, "Slow") || strstr(issue, "Bug") || strstr(issue, "Glitch")) {
        strcpy(priorityStr, "Medium");
    } else {
        strcpy(priorityStr, "Low");
    }
}

int autoCleanup() {
    if (isEmpty()) return 0;
    struct Ticket temp[MAX];
    int count = 0, freed = 0;
    int i = front;
    while (1) {
        if (strcmp(queue[i].status, "Closed") != 0) {
            temp[count++] = queue[i];
        } else {
            freed++;
        }
        if (i == rear) break;
        i = (i + 1) % MAX;
    }
    if (freed == 0) return 0;
    for (int j = 0; j < count; j++) queue[j] = temp[j];
    front = 0;
    rear = count - 1;
    return freed;
}

void saveToFile() {
    FILE *file = fopen("final_dataset_matched.csv", "w");
    if (!file) return;
    fprintf(file, "Ticket ID,Customer Name,Customer Email,Product Purchased,Date of Purchase,Ticket Type,Ticket Status,Ticket Priority,First Response Time,Time to Resolution\n");
    if (!isEmpty()) {
        int i = front;
        while (1) {
            fprintf(file, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    queue[i].ticketID, queue[i].customerName, queue[i].email, 
                    queue[i].product, queue[i].dateOfPurchase, queue[i].issueType,
                    queue[i].status, queue[i].priority, queue[i].responseTime, queue[i].resolveTime);
            if (i == rear) break;
            i = (i + 1) % MAX;
        }
    }
    fclose(file);
}

void generateAdminHTML() {
    FILE *file = fopen("admin_view.html", "w");
    if (!file) return;

    fprintf(file, "<!DOCTYPE html><html><head><title>Admin Panel</title>");
    fprintf(file, "<style>body{font-family:'Segoe UI', sans-serif; padding:20px; background:#f4f7f6;} table{width:100%%; border-collapse:collapse; background:white; box-shadow:0 2px 10px rgba(0,0,0,0.1);} th{background:#2c3e50; color:white;} th,td{padding:12px; border-bottom:1px solid #ddd; text-align:left;} .btn{background:#e74c3c; color:white; padding:6px 12px; text-decoration:none; border-radius:4px; font-size:12px;} .btn:hover{background:#c0392b;} .closed{background:#f9f9f9; color:#999;} .badge{padding:4px 8px; border-radius:10px; color:white; font-size:11px;} .Critical{background:#e74c3c;} .High{background:#e67e22;} .Medium{background:#f1c40f; color:black;} .Low{background:#2ecc71;}</style>");
    fprintf(file, "<meta http-equiv='refresh' content='600'>"); // Refresh every 10min
    fprintf(file, "</head><body>");
    
    fprintf(file, "<div style='display:flex; justify-content:space-between; align-items:center;'><h1>🛡️ Admin Control Panel</h1><a href='/logout' style='color:red; text-decoration:none; font-weight:bold;'>Logout ➜</a></div>");
    
    if (!isEmpty()) {
        fprintf(file, "<table><thead><tr><th>ID</th><th>User</th><th>Issue</th><th>Priority</th><th>Status</th><th>Action</th></tr></thead><tbody>");
        int i = front;
        while (1) {
            fprintf(file, "<tr class='%s'>", queue[i].status);
            fprintf(file, "<td>#%d</td><td>%s<br><small>%s</small></td><td>%s</td>", queue[i].ticketID, queue[i].customerName, queue[i].email, queue[i].issueType);
            fprintf(file, "<td><span class='badge %s'>%s</span></td><td>%s</td>", queue[i].priority, queue[i].priority, queue[i].status);
            
            if (strcmp(queue[i].status, "Open") == 0) {
                fprintf(file, "<td><a href='/resolve/%d' class='btn'>Mark Resolved</a></td>", queue[i].ticketID);
            } else {
                fprintf(file, "<td>✅ Done</td>");
            }
            fprintf(file, "</tr>");
            if (i == rear) break;
            i = (i + 1) % MAX;
        }
        fprintf(file, "</tbody></table>");
    } else {
        fprintf(file, "<p>No active tickets found.</p>");
    }
    fprintf(file, "</body></html>");
    fclose(file);
}

void loadFromFile() {
    FILE *file = fopen("customer_support_tickets_updated.csv", "r");
    if (!file) {
        printf("[INIT] No database file found. Starting fresh.\n");
        return;
    }

    char line[1024];
    fgets(line, sizeof(line), file); 

    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        removeNewline(line);

        if (strlen(line) < 5) continue;

        struct Ticket t;

        char *token = strtok(line, ",");
        if (!token) continue; 
        t.ticketID = atoi(token);

        token = strtok(NULL, ","); if(token) strcpy(t.customerName, token); else strcpy(t.customerName, "Unknown");
        token = strtok(NULL, ","); if(token) strcpy(t.email, token); else strcpy(t.email, "N/A");
        token = strtok(NULL, ","); if(token) strcpy(t.product, token); else strcpy(t.product, "N/A");
        token = strtok(NULL, ","); if(token) strcpy(t.dateOfPurchase, token); else strcpy(t.dateOfPurchase, "N/A");
        token = strtok(NULL, ","); if(token) strcpy(t.issueType, token); else strcpy(t.issueType, "No Description");
        token = strtok(NULL, ","); if(token) strcpy(t.status, token); else strcpy(t.status, "Open");
        token = strtok(NULL, ","); if(token) strcpy(t.priority, token); else strcpy(t.priority, "Low");
        token = strtok(NULL, ","); if(token) strcpy(t.responseTime, token); else strcpy(t.responseTime, "N/A");
        token = strtok(NULL, ","); if(token) strcpy(t.resolveTime, token); else strcpy(t.resolveTime, "N/A");

        if (isFull()) {
            autoCleanup(); 
        }
        
        if (front == -1) front = 0;
        rear = (rear + 1) % MAX;
        queue[rear] = t;
        count++;
    }
    
    fclose(file);

    generateAdminHTML(); 
    
    printf("==========================================\n");
    printf(" [INIT] Database Loaded Successfully!\n");
    printf(" Total Tickets in Memory: %d\n", count);
    printf("==========================================\n");
}

void processWebTickets() {
    FILE *file = fopen("pending_tickets.csv", "r");
    if (!file) return;
    char line[1000];
    int newCount = 0;
    while (fgets(line, sizeof(line), file)) {
        removeNewline(line);
        if (strlen(line) < 5) continue;
        struct Ticket t;
        t.ticketID = (rear == -1) ? 1001 : queue[rear].ticketID + 1;
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,\n]", 
               t.customerName, t.email, t.product, t.dateOfPurchase, t.issueType);
        getAutoPriority(t.issueType, t.priority);
        strcpy(t.status, "Open");
        getCurrentTime(t.responseTime);
        strcpy(t.resolveTime, "Pending");
        
        if (isFull()) autoCleanup();
        if (front == -1) front = 0;
        rear = (rear + 1) % MAX;
        queue[rear] = t;
        newCount++;
        printf("[WEB] New Ticket #%d added.\n", t.ticketID);
    }
    fclose(file);
    if (newCount > 0) {
        file = fopen("pending_tickets.csv", "w"); fclose(file);
        generateAdminHTML();
        saveToFile();
    }
}

void processAdminCommands() {
    FILE *file = fopen("admin_commands.txt", "r");
    if (!file) return;

    char line[100];
    int updateNeeded = 0;

    while (fgets(line, sizeof(line), file)) {

        int targetID = -1;

        if (sscanf(line, "RESOLVE,%d", &targetID) == 1) {
            
            printf("[ADMIN REQUEST] Request to Close Ticket #%d\n", targetID);
            
            int found = 0;
            if (!isEmpty()) {
                int i = front;
                while (1) {
                    if (queue[i].ticketID == targetID) {
                        if (strcmp(queue[i].status, "Closed") != 0) {
                            strcpy(queue[i].status, "Closed");
                            getCurrentTime(queue[i].resolveTime);
                            printf("✅ SUCCESS: Ticket #%d is now CLOSED.\n", targetID);
                            updateNeeded = 1;
                        } else {
                            printf("⚠️ INFO: Ticket #%d was already closed.\n", targetID);
                        }
                        found = 1;
                        break; 
                    }
                    if (i == rear) break;
                    i = (i + 1) % MAX;
                }
            }
            if (!found) {
                printf("❌ ERROR: Ticket ID #%d not found in system.\n", targetID);
            }
        }
    }
    fclose(file);

    file = fopen("admin_commands.txt", "w");
    fclose(file);

    if (updateNeeded) {
        generateAdminHTML(); 
        saveToFile();        
        printf("💾 System Updated.\n");
    }
}

void startServerMode() {
    printf("Backend Active. Listening...\n");
    while (1) {
        processWebTickets();
        processAdminCommands();
        SLEEP_MS(2000);
    }
}

int main() {
    loadFromFile();
    startServerMode(); 
    return 0;
}