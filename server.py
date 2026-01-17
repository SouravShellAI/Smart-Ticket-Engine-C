from flask import Flask, render_template, request, redirect, url_for, flash
import csv
import os
import time
import re 

app = Flask(__name__, template_folder='templates', static_folder='static')
app.secret_key = 'super_secret_key'

def init_db():
    if not os.path.exists('users.csv'):
        with open('users.csv', 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['username', 'password']) 
init_db()

def is_valid_input(text):
    pattern = r"^[a-zA-Z_ ]{3,20}$"
    if not re.match(pattern, text):
        return False

    lower_text = text.lower().strip()

    banned_words = [
        "abc", "xyz", "admin", "root", "user", "guest", "test", 
        "qwerty", "asdf", "dummy", "null", "void", "login", 
        "signup", "support", "help", "bot", "fake"
    ]

    if lower_text in banned_words:
        return False

    if re.search(r"(.)\1{2,}", lower_text):
        return False

    return True

def check_user(username, password):
    if not os.path.exists('users.csv'): return False
    with open('users.csv', 'r') as f:
        reader = csv.reader(f)
        next(reader, None)
        for row in reader:
            if row and row[0] == username and row[1] == password:
                return True
    return False

def register_user(username, password):
    if check_user(username, password): return False
    with open('users.csv', 'a', newline='') as f:
        csv.writer(f).writerow([username, password])
    return True

def get_next_ticket_id():
    max_id = 1000 

    if os.path.exists('customer_support_tickets_updated.csv'):
        with open('customer_support_tickets_updated.csv', 'r') as f:
            reader = csv.reader(f)
            next(reader, None) 
            for row in reader:
                if row:
                    try:
                        curr_id = int(row[0])
                        if curr_id > max_id: max_id = curr_id
                    except: pass
    
    if os.path.exists('pending_tickets.csv'):
        with open('pending_tickets.csv', 'r') as f:
            reader = csv.reader(f)
            for row in reader:
                if row:
                    try:
                        curr_id = int(row[0])
                        if curr_id > max_id: max_id = curr_id
                    except: pass
                    
    return max_id + 1

@app.route('/')
def landing(): return render_template('index.html')

@app.route('/login')
def login_page(): return render_template('login.html')

@app.route('/signup')
def signup_page(): return render_template('signup.html')

@app.route('/do_signup', methods=['POST'])
def do_signup():
    username = request.form['username']
    password = request.form['password']

    if not is_valid_input(username):
        flash("Invalid Username! No numbers or special characters allowed. Only letters (A-Z) and underscore (_).", "error")
        return redirect(url_for('signup_page'))

    if register_user(username, password):
        flash("Account created! Please login.", "success")
        return redirect(url_for('login_page'))
    else:
        flash("Username already exists!", "error")
        return redirect(url_for('signup_page'))

@app.route('/auth', methods=['POST'])
def auth():
    username = request.form['username']
    password = request.form['password']
    role = request.form.get('role', 'user')

    if role == 'admin':
        if username == "admin" and password == "admin123":
            return redirect(url_for('admin_dashboard'))
        else:
            flash("Invalid Admin Password!", "error")
            return redirect(url_for('login_page'))
    else:
        if check_user(username, password):
            return redirect(url_for('user_homepage'))
        else:
            flash("Invalid Username or Password!", "error")
            return redirect(url_for('login_page'))

@app.route('/home')
def user_homepage(): return render_template('homepage.html')

@app.route('/index1')
def ticket_form(): return render_template('index1.html')

@app.route('/submit_ticket', methods=['POST'])
def submit_ticket():
    name = request.form['name']
    email = request.form['email']
    product = request.form['product']
    dop = request.form['dop'] 
    desc = request.form['description']

    new_ticket_id = get_next_ticket_id()

    with open('pending_tickets.csv', 'a', newline='') as f:
        csv.writer(f).writerow([new_ticket_id, name, email, product, dop, desc])

    return render_template('homepage.html', 
                           message=f"Success! Your Ticket ID is: {new_ticket_id}", 
                           new_id=new_ticket_id)

@app.route('/status.html')
def status_page(): return render_template('status.html')

@app.route('/check_status', methods=['POST'])
def check_status():
    ticket_id = request.form.get('ticket_id').strip()
    email_input = request.form.get('email').strip().lower()
    
    found_status = None
    found_customer = None
    found_issue = None
    found_dop = None          
    found_resolve_time = None
    error_msg = None

    def search_csv(filename, db_type):
        if not os.path.exists(filename): return None
        with open(filename, 'r') as f:
            reader = csv.reader(f)
            for row in reader:
                
                if len(row) < 6: continue 
                
                csv_id = row[0].strip()
                csv_email = row[2].strip().lower()

                if csv_id == ticket_id:
                    if csv_email == email_input:
                        return row
                    else:
                        return "UNAUTHORIZED"
        return None

    result = search_csv('customer_support_tickets_updated.csv', 'active')
    if result == "UNAUTHORIZED":
        error_msg = "Security Error: Ticket ID exists but Email does not match!"
    elif result:
        found_status = "Open"
        found_customer = result[1]
        found_dop = result[4]   
        found_issue = result[5]   

    if not found_status and not error_msg:
        result = search_csv('resolved_tickets.csv', 'resolved')
        if result == "UNAUTHORIZED":
            error_msg = "Security Error: Ticket ID exists but Email does not match!"
        elif result:
            found_status = "Resolved"
            found_customer = result[1]
            found_dop = result[4]     
            found_issue = result[5]    
            found_resolve_time = result[9] if len(result) > 9 else "N/A"

    if not found_status and not error_msg:
        error_msg = "Ticket ID not found in our database."

    return render_template('status.html', 
                           ticket_id=ticket_id if not error_msg else None, 
                           status=found_status, 
                           customer=found_customer, 
                           dop=found_dop, 
                           issue=found_issue, 
                           resolve_time=found_resolve_time, 
                           error=error_msg)

@app.route('/admin')
def admin_dashboard():
    if os.path.exists('templates/admin_view.html'): return render_template('admin_view.html')
    return "<h3>Dashboard loading... refresh shortly.</h3>"

@app.route('/resolve/<int:ticket_id>')
def resolve_ticket(ticket_id):
    with open('admin_commands.txt', 'w') as f: f.write(f"RESOLVE {ticket_id}")
    time.sleep(1)
    return redirect(url_for('admin_dashboard'))

if __name__ == '__main__':
    app.run(debug=True, port=5000)
