#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>     
#include <unistd.h>   

#define MAX 10000    

struct Ticket {
    int ticketID;
    char customerName[100];
    char email[100];
    char product[100];
    char dateOfPurchase[50]; 
    char issueType[200];    
    char status[20];
    char priority[20];
    char responseTime[50];
    char resolveTime[50];
};

struct Ticket queue[MAX];
int front = -1;
int rear = -1;
int max_id = 1000; 

void removeNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') str[len - 1] = '\0';
}

int isFull() { return (rear + 1) % MAX == front; }
int isEmpty() { return front == -1; }

void getSystemTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
}

const char* getAutoPriority(const char* description) {
    char lowerDesc[300];
    strcpy(lowerDesc, description);
    for(int i = 0; lowerDesc[i]; i++) lowerDesc[i] = tolower(lowerDesc[i]);

    if (strstr(lowerDesc, "hack") || strstr(lowerDesc, "money") || strstr(lowerDesc, "security") || strstr(lowerDesc, "crash")) return "Critical";
    else if (strstr(lowerDesc, "error") || strstr(lowerDesc, "fail") || strstr(lowerDesc, "urgent")) return "High";
    else if (strstr(lowerDesc, "slow") || strstr(lowerDesc, "bug")) return "Medium";
    return "Low";
}

void loadFromFile() {
    FILE *file = fopen("customer_support_tickets_updated.csv", "r");
    if (!file) {
        file = fopen("customer_support_tickets_updated.csv", "w");
        fprintf(file, "Ticket ID,Customer Name,Customer Email,Product Purchased,Date of Purchase,Issue Description,Ticket Status,Ticket Priority,First Response Time,Time to Resolution\n");
        fclose(file);
        return;
    }

    char line[1024];
    fgets(line, sizeof(line), file);

    front = -1; rear = -1;

    while (fgets(line, sizeof(line), file)) {
        removeNewline(line);
        if (strlen(line) < 5) continue;

        struct Ticket t;
        char *token = strtok(line, ","); if(!token) continue; t.ticketID = atoi(token);
        if (t.ticketID > max_id) max_id = t.ticketID;

        token = strtok(NULL, ","); strcpy(t.customerName, token ? token : "Unknown");
        token = strtok(NULL, ","); strcpy(t.email, token ? token : "N/A");
        token = strtok(NULL, ","); strcpy(t.product, token ? token : "N/A");

        token = strtok(NULL, ","); strcpy(t.dateOfPurchase, token ? token : "N/A");

        token = strtok(NULL, ","); strcpy(t.issueType, token ? token : "No Desc");
        
        token = strtok(NULL, ","); strcpy(t.status, token ? token : "Open");
        token = strtok(NULL, ","); strcpy(t.priority, token ? token : "Low");
        token = strtok(NULL, ","); strcpy(t.responseTime, token ? token : "N/A");
        token = strtok(NULL, ","); strcpy(t.resolveTime, token ? token : "N/A");

        if (front == -1) front = 0;
        rear = (rear + 1) % MAX;
        queue[rear] = t;
    }
    fclose(file);
}

void archiveAndRemove(int idToResolve) {
    FILE *srcFile = fopen("customer_support_tickets_updated.csv", "r");
    FILE *tempFile = fopen("temp_dataset.csv", "w");
    FILE *archiveFile = fopen("resolved_tickets.csv", "a"); 
    
    if (!srcFile || !tempFile || !archiveFile) {
        printf("Error opening files!\n");
        return;
    }

    fseek(archiveFile, 0, SEEK_END);
    if (ftell(archiveFile) == 0) {
        fprintf(archiveFile, "Ticket ID,Customer Name,Customer Email,Product Purchased,Date of Purchase,Issue Description,Ticket Status,Ticket Priority,First Response Time,Time to Resolution\n");
    }

    char line[1024];
    char lineCopy[1024]; 

    if (fgets(line, sizeof(line), srcFile)) fprintf(tempFile, "%s", line); 

    while (fgets(line, sizeof(line), srcFile)) {
        removeNewline(line);
        strcpy(lineCopy, line);

        char *token = strtok(lineCopy, ",");
        int currentID = atoi(token);

        if (currentID == idToResolve) {
            char name[100], email[100], prod[100], dop[50], issue[200], prio[20];
            
            token = strtok(NULL, ","); strcpy(name, token ? token : "");
            token = strtok(NULL, ","); strcpy(email, token ? token : "");
            token = strtok(NULL, ","); strcpy(prod, token ? token : "");
            token = strtok(NULL, ","); strcpy(dop, token ? token : "");
            token = strtok(NULL, ","); strcpy(issue, token ? token : "");
            token = strtok(NULL, ","); 
            token = strtok(NULL, ","); strcpy(prio, token ? token : "");
            
            char timeBuf[50];
            getSystemTime(timeBuf); 

            fprintf(archiveFile, "%d,%s,%s,%s,%s,%s,Resolved,%s,N/A,%s\n",
                    currentID, name, email, prod, dop, issue, prio, timeBuf);
            
            printf("[ARCHIVE] Ticket %d moved to resolved_tickets.csv\n", currentID);

        } else {
            fprintf(tempFile, "%s\n", line); 
        }
    }

    fclose(srcFile);
    fclose(tempFile);
    fclose(archiveFile);

    remove("customer_support_tickets_updated.csv");
    rename("temp_dataset.csv", "customer_support_tickets_updated.csv");
}

void resolveTicket(int id) {
    archiveAndRemove(id);
    loadFromFile();
}

void processPendingTickets() {
    FILE *pendingFile = fopen("pending_tickets.csv", "r");
    if (!pendingFile) return;

    FILE *dbFile = fopen("customer_support_tickets_updated.csv", "a");
    char line[1024];

    while (fgets(line, sizeof(line), pendingFile)) {
        removeNewline(line);
        if (strlen(line) < 3) continue;

        char name[100], email[100], product[100], dop[50], desc[200];
        int pendingID; 

        char *token = strtok(line, ",");
        if(!token) continue; pendingID = atoi(token);
        if (pendingID > max_id) max_id = pendingID;

        token = strtok(NULL, ","); strcpy(name, token ? token : "Unknown");
        token = strtok(NULL, ","); strcpy(email, token ? token : "N/A");
        token = strtok(NULL, ","); strcpy(product, token ? token : "N/A");

        token = strtok(NULL, ","); strcpy(dop, token ? token : "N/A");

        token = strtok(NULL, ","); strcpy(desc, token ? token : "No Desc");

        const char* priority = getAutoPriority(desc);

        fprintf(dbFile, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                pendingID, name, email, product, dop, desc, "Open", priority, "N/A", "N/A");
        
        printf("[NEW] Ticket #%d added (DOP: %s).\n", pendingID, dop);
    }
    fclose(pendingFile);
    fclose(dbFile);
    
    pendingFile = fopen("pending_tickets.csv", "w");
    fclose(pendingFile);
    loadFromFile(); 
}

void generateAdminHTML() {
    FILE *file = fopen("templates/admin_view.html", "w"); 
    if (!file) return;

    fprintf(file, "<html><head><title>Admin Dashboard</title>");
    fprintf(file, "<style>");
    fprintf(file, "body { font-family: 'Segoe UI', sans-serif; background: #f4f6f9; padding: 20px; }");
    fprintf(file, "table { width: 100%%; border-collapse: collapse; background: white; box-shadow: 0 4px 8px rgba(0,0,0,0.1); border-radius: 8px; overflow: hidden; }");
    fprintf(file, "th, td { padding: 15px; text-align: left; border-bottom: 1px solid #ddd; vertical-align: middle; }");
    fprintf(file, "th { background-color: #2c3e50; color: white; text-transform: uppercase; font-size: 13px; letter-spacing: 0.5px; }");
    fprintf(file, "tr:hover { background-color: #f8f9fa; }");
    fprintf(file, ".Critical { color: #c0392b; font-weight: bold; background: #fadbd8; padding: 4px 8px; border-radius: 4px; font-size: 12px; }");
    fprintf(file, ".High { color: #e67e22; font-weight: bold; background: #fdebd0; padding: 4px 8px; border-radius: 4px; font-size: 12px; }");
    fprintf(file, ".Medium { color: #2980b9; background: #d6eaf8; padding: 4px 8px; border-radius: 4px; font-size: 12px; }");
    fprintf(file, ".Low { color: #27ae60; background: #d5f5e3; padding: 4px 8px; border-radius: 4px; font-size: 12px; }");
    fprintf(file, ".btn { background: #27ae60; color: white; padding: 8px 12px; text-decoration: none; border-radius: 4px; font-size: 13px; font-weight: 600; transition: 0.3s; }");
    fprintf(file, ".btn:hover { background: #219150; box-shadow: 0 2px 5px rgba(0,0,0,0.2); }");
    fprintf(file, ".logout-btn { float: right; background: #e74c3c; color: white; padding: 10px 20px; text-decoration: none; border-radius: 30px; font-weight: bold; font-size: 14px; box-shadow: 0 2px 5px rgba(231, 76, 60, 0.3); }");
    fprintf(file, ".logout-btn:hover { background: #c0392b; transform: translateY(-2px); }");
    fprintf(file, ".subtext { display: block; font-size: 12px; color: #7f8c8d; margin-top: 4px; }");
    fprintf(file, "</style>");
    fprintf(file, "<meta http-equiv='refresh' content='15'>"); 
    fprintf(file, "</head><body>");
    
    fprintf(file, "<div style='overflow: hidden; margin-bottom: 20px;'>");
    fprintf(file, "<a href='/' class='logout-btn'>Logout</a>");
    fprintf(file, "<h2 style='color: #2c3e50; margin: 0;'>🚀 Live Support Dashboard</h2>");
    fprintf(file, "<p style='color: #7f8c8d; margin: 5px 0 0 0;'>Real-time ticket monitoring system</p>");
    fprintf(file, "</div>");

    fprintf(file, "<table>");
    fprintf(file, "<tr><th width='5%%'>ID</th><th width='20%%'>Customer Details</th><th width='20%%'>Product Info</th><th width='30%%'>Issue Description</th><th width='10%%'>Priority</th><th width='15%%'>Action</th></tr>");

    if (!isEmpty()) {
        int i = front;
        while (1) {
            fprintf(file, "<tr>");
            fprintf(file, "<td><strong>#%d</strong></td>", queue[i].ticketID);
            
            fprintf(file, "<td><span style='font-weight:600; color:#2c3e50;'>%s</span><span class='subtext'>✉️ %s</span></td>", queue[i].customerName, queue[i].email);

            fprintf(file, "<td><span style='font-weight:600; color:#2c3e50;'>%s</span><span class='subtext'>📅 %s</span></td>", queue[i].product, queue[i].dateOfPurchase);

            fprintf(file, "<td>%s</td>", queue[i].issueType);
            
            fprintf(file, "<td><span class='%s'>%s</span></td>", queue[i].priority, queue[i].priority);
            fprintf(file, "<td><a href='/resolve/%d' class='btn'>Mark Resolve ✅</a></td>", queue[i].ticketID);
            fprintf(file, "</tr>");

            if (i == rear) break;
            i = (i + 1) % MAX;
        }
    } else {
        fprintf(file, "<tr><td colspan='6' style='text-align:center; padding: 40px; color: #95a5a6;'><h3>No Pending Tickets! 🎉</h3><p>Good job team, all caught up.</p></td></tr>");
    }

    fprintf(file, "</table>");
    fprintf(file, "<div style='text-align:center; margin-top:20px; color:#bdc3c7; font-size:12px;'>System Auto-Refreshes every 15s</div>");
    fprintf(file, "</body></html>");
    fclose(file);
}

void checkAdminCommands() {
    FILE *cmdFile = fopen("admin_commands.txt", "r");
    if (cmdFile) {
        int idToResolve;
        if (fscanf(cmdFile, "RESOLVE %d", &idToResolve) == 1) {
            printf("[CMD] Resolving ID: %d\n", idToResolve);
            resolveTicket(idToResolve); 
            generateAdminHTML(); 
        }
        fclose(cmdFile);
        cmdFile = fopen("admin_commands.txt", "w");
        fclose(cmdFile);
    }
}

int main() {
    printf("==========================================\n");
    printf("   DSA TICKETING ENGINE (C BACKEND)       \n");
    printf("==========================================\n");

    loadFromFile();      
    generateAdminHTML(); 

    while (1) {
        processPendingTickets(); 
        checkAdminCommands();    
        generateAdminHTML();     
        sleep(2); 
    }
    return 0;
}
