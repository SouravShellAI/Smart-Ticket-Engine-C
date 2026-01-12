# Customer Support Ticketing System

## Overview
This project demonstrates a real-world implementation of **Data Structures and Algorithms (DSA)**. While traditional C programs run on a console, this project bridges the gap between **System-Level C Logic** and **Modern Web Interfaces**.

We use **Python (Flask)** strictly as a middleware to capture user input from a browser, while the core processing—including **Circular Queue Management**, **Priority Assignment Algorithms**, and **Database Handling**—is executed entirely in **C**.

---

## Key Features

### 1. Hybrid Architecture (C + Python)
- **Python (The Body):** Acts as a bridge to send HTML form data to the backend.
- **C (The Brain):** Handles the entire logic, memory management, and file operations.

### 2. Smart Priority Algorithm (DSA Logic)
The system automatically assigns priority (`Critical`, `High`, `Medium`, `Low`) based on keyword analysis using C string manipulation functions (`strstr`).
- *Example:* "My account was **hacked**" → Auto-tagged as **CRITICAL**.

### 3. Circular Queue Implementation
- Uses a **Circular Queue** data structure in C to manage active tickets efficiently in memory (RAM) before saving them to the database.

### 4. Real-Time Admin Dashboard
- Admins can view live tickets and mark them as "Resolved". The status updates instantly across the database using IPC (Inter-Process Communication).

---

# How to Run Locally
Follow these steps to set up the project on your machine.

Prerequisites
GCC Compiler (MinGW for Windows / GCC for Linux/Mac).
Python 3.x installed.

Step 1: Install Dependencies
Open a terminal in the project folder and run:

pip install -r requirements.txt

Step 2: Start the Web Server (Terminal 1)
Open a terminal and run the Python server. This handles the browser interface.

python server.py

#Keep this terminal running.

Step 3: Start the Backend Engine (Terminal 2)
Open a NEW terminal (Split Terminal) and compile/run the C code. This processes the tickets.

gcc main.c -o ticketing_system

# Run (Windows)
./ticketing_system.exe
(On Mac/Linux use ./ticketing_system)

Step 4: Access the Application
Open your browser and visit:

Customer Portal: http://localhost:5000

Admin Dashboard: http://localhost:5000/admin

Admin Password: admin123
