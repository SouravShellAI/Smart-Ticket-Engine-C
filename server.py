import csv
from flask import Flask, request, redirect, session
from datetime import datetime
import os

app = Flask(__name__)
app.secret_key = 'super_secret_key'

@app.route('/')
def home():
    if os.path.exists('index.html'): 
        return open('index.html', encoding='utf-8').read()
    return "<h1>Error: index.html missing!</h1>"

@app.route('/submit', methods=['POST'])
def submit_ticket():
    name = request.form.get('name')
    email = request.form.get('email')
    product = request.form.get('product')
    issue = request.form.get('issue')
    date = datetime.now().strftime("%Y-%m-%d")

    with open('pending_tickets.csv', 'a', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow([name, email, product, date, issue])

    return f"""
    <div style='text-align:center; font-family:sans-serif; padding:50px;'>
        <h1 style='color:green'>✅ Ticket Submitted Successfully!</h1>
        <p>Your issue regarding <b>{product}</b> has been sent to our System.</p>
        <a href='/' style='background:#667eea; color:white; padding:10px 20px; text-decoration:none; border-radius:5px;'>Submit Another</a>
    </div>
    """

@app.route('/admin', methods=['GET', 'POST'])
def admin_login():
    if request.method == 'POST':
        if request.form.get('pass') == 'admin123':
            session['logged_in'] = True
            return redirect('/admin/dashboard')
        else:
            return "<h1>Wrong Password! <a href='/admin'>Try Again</a></h1>"
    return """
    <div style='display:flex; justify-content:center; align-items:center; height:100vh; background:#f4f7f6; font-family:sans-serif;'>
        <div style='background:white; padding:40px; border-radius:10px; box-shadow:0 10px 25px rgba(0,0,0,0.1); text-align:center;'>
            <h2>🔒 Admin Login</h2>
            <form method='POST'>
                <input type='password' name='pass' placeholder='Password' style='padding:10px; width:200px; margin-bottom:10px;' required><br>
                <button type='submit' style='padding:10px 20px; background:#2c3e50; color:white; border:none; cursor:pointer;'>Login</button>
            </form>
        </div>
    </div>
    """

@app.route('/admin/dashboard')
def admin_dashboard():
    if not session.get('logged_in'): return redirect('/admin')
    
    content = "<h2 style='text-align:center;'>⏳ Waiting for Backend...</h2><script>setTimeout(function(){location.reload()}, 2000);</script>"
    
    if os.path.exists('admin_view.html'): 
        try:
            with open('admin_view.html', 'r', encoding='utf-8') as f:
                content = f.read()
        except Exception as e:
            print("Error reading file:", e)

    from flask import make_response
    response = make_response(content)
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Pragma"] = "no-cache"
    response.headers["Expires"] = "0"
    
    return response

@app.route('/resolve/<id>')
def resolve_ticket(id):
    if not session.get('logged_in'): return redirect('/admin')

    with open('admin_commands.txt', 'a', encoding='utf-8') as f:
        f.write(f"RESOLVE,{id}\n")
        f.flush()
        os.fsync(f.fileno()) 
        
    print(f"DEBUG: Written command RESOLVE,{id} to file.") 
    return "<script>window.location.href = '/admin/dashboard';</script>"

@app.route('/logout')
def logout():
    session.clear()
    return redirect('/admin')

if __name__ == '__main__':
    print("🌐 Web Server Running (Manual Mode)")
    app.run(port=5000)