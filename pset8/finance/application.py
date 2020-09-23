import os
import datetime as dt

from cs50 import SQL
from flask import Flask, flash, jsonify, redirect, render_template, request, session
from flask_session import Session
from tempfile import mkdtemp
from werkzeug.exceptions import default_exceptions, HTTPException, InternalServerError
from werkzeug.security import check_password_hash, generate_password_hash


from helpers import apology, login_required, lookup, usd

# Configure application
app = Flask(__name__)

# Ensure templates are auto-reloaded
app.config["TEMPLATES_AUTO_RELOAD"] = True

# Ensure responses aren't cached
@app.after_request
def after_request(response):
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response

# Custom filter
app.jinja_env.filters["usd"] = usd

# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_FILE_DIR"] = mkdtemp()
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///finance.db")

# Make sure API key is set
if not os.environ.get("API_KEY"):
    raise RuntimeError("API_KEY not set")

# Add a new table called transactions to keep a historic record
db.execute("CREATE TABLE IF NOT EXISTS 'transactions' "
        + "('user_id' INTEGER NOT NULL"
        + ", 'symbol' TEXT NOT NULL"
        + ", 'shares' NUMERIC NOT NULL"
        + ", 'price' NUMERIC NOT NULL"
        + ", 'date' TIMESTAMP NOT NULL"
        + ", FOREIGN KEY (user_id) REFERENCES users(id))")

# Create an index on the FK column of transactions
db.execute("CREATE INDEX IF NOT EXISTS 'transactions_user_id_index'"
        + " ON transactions(user_id)")


# Get the cash of the received user
def getCash(user_id):

    cash = db.execute("SELECT * FROM users WHERE id = :user_id", user_id = user_id)
    cash = float(cash[0]["cash"])
    return cash


# Get all the data about the received user
def getUser(user_id):
    return db.execute("SELECT * FROM users"
                    + " WHERE (users.id = :user_id)"
                    , user_id=user_id)


# Get the shares of the received symbol the received user has, if any
def getShares(user_id, symbol):
    shares = db.execute("SELECT SUM(shares) as shares"
                + " FROM transactions"
                + " WHERE transactions.user_id = :user_id"
                + " AND transactions.symbol LIKE :symbol"
                , user_id = user_id
                , symbol = symbol)
    return shares[0]["shares"]


# Get all the symbols the received user has bought
def getSymbols(user_id):
    return db.execute("SELECT DISTINCT(symbol)"
                    + " FROM transactions"
                    + " WHERE user_id = :user_id"
                    , user_id = user_id)


def getTransactions(user_id):
    return db.execute("SELECT symbol, shares, price, date as transacted"
                    + " FROM transactions WHERE user_id = :user_id "
                    + " ORDER BY date DESC"
                    , user_id=user_id)


# Add a new user to 'users' table
def newUser(username, hashed_password):
    db.execute("INSERT INTO users(username, hash) "
            + "VALUES (:username, :hashed)"
            , username = username
            , hashed = hashed_password)
    return


# Add a new transaction to 'transactions' table
def newTransaction(user_id, symbol, shares, price, date):
    db.execute("INSERT INTO transactions (user_id, symbol, shares, price, date)"
            + " VALUES (:user_id, :symbol, :shares, :price, :date)"
            , user_id = user_id
            , symbol = symbol
            , shares = shares
            , price = price
            , date = date)
    return


# Update the cash of the received user
def updateCash(user_id, new_cash):
    db.execute("UPDATE users"
            + " SET cash = :new_cash WHERE id = :user_id"
            , new_cash = new_cash
            , user_id = user_id)
    return

# Update the password of the received user
def updatePassword(user_id, new_password):

    # Get the hashed value
    hashed_password = generate_password_hash(new_password)

    # Update the users table
    db.execute("UPDATE users"
            + " SET hash = :hashed_password"
            + " WHERE id = :user_id"
            , hashed_password = hashed_password
            , user_id = user_id)

    return


def verify_password(request):
    """ Verify the password integrity"""

    password = request.form.get("password")
    confirmation = request.form.get("confirmation")

    # Ensure password and confirmation aren't blank
    if not password:
        return False, "Missing password"
    if not confirmation:
        return False, "Missing password confirmation"

    # Ensure password length
    if len(password) < 4:
        return False, "Password at least 4 chars long!"

    # Ensure password and confirmation match
    if password != confirmation:
        return False, "Passwords must match!"

    # Initialize accums
    alpha = 0
    dec = 0
    sym = 0

    # Ensure it has at least a number and a symbol
    for char in password:
        print(f"PROCESSING CHAR={char}...")
        if char.isalpha():
            alpha += 1
        if char.isdecimal():
            dec += 1
        if char.isprintable() and not char.isalnum():
            sym += 1
    # Endfor

    if not alpha >= 4:
        return False, "Must have at least 4 letters!"

    if not dec >= 2:
        return False, "Must have at least 2 numbers!"

    if not sym >= 1:
        return False, "Must have at least 1 symbol"

    return True, None


@app.route("/")
@login_required
def index():
    """Show portfolio of stocks"""

    # Get symbol, shares and cash of current user from DB
    # as a list of dicts per row in DB
    transactions = db.execute("SELECT transactions.symbol as symbol"
                        + ", SUM(transactions.shares) as shares"
                        + " FROM transactions"
                        + " WHERE transactions.user_id = :user_id"
                        + " GROUP BY symbol"
                        , user_id=session["user_id"])

    # Initialize to 0
    total = 0

    # For every dict in the list, which maps to every row in DB's table
    for transaction in transactions:

        # Get the symbol and perform the lookup
        res = lookup(transaction["symbol"])

        # Remove the shares
        shares = transaction.pop("shares")

        # Insert in second place the symbol's name
        transaction["name"] = res["name"]

        # Re-insert shares after "name"
        transaction["shares"] = shares

        # Append the symbol's current price
        transaction["price"] = res["price"]

        # Calculate how much money the user has
        total += shares * res["price"]

        # Add total to the dict
        transaction["total"] = total

    # Get current user's available cash
    cash = getCash(session["user_id"])

    return render_template("index.html", rows=transactions, total=total, cash=cash)



@app.route("/add_cash", methods=["GET", "POST"])
@login_required
def add_cash():
    """Add cash to the account"""

    if request.method == "GET":
        return render_template("add_cash.html")
    else:

        # Ensure cash is not blank
        if not request.form.get("cash"):
            return apology("Must enter an amount")

        # Get the amount
        to_add = float(request.form.get("cash"))

        # Ensure it's a valid amount
        if not to_add > 0:
            return apology("Must enter a positive value")

        # Get user cash
        user_cash = getCash(session["user_id"])

        # Update user cash
        updateCash(session["user_id"], user_cash + to_add)

        # Inform success
        flash("Cash added!")

        return redirect("/")

@app.route("/buy", methods=["GET", "POST"])
@login_required
def buy():
    """Buy shares of stock"""
    if request.method == "GET":
        return render_template("buy.html")
    else:

        # Ensure symbol and shares are not blank
        if not request.form.get("symbol"):
            return apology("Must enter a stock symbol")
        if not request.form.get("shares"):
            return apology("Must enter an amount of shares to buy")

        # Get the symbol and shares from the form
        symbol = request.form.get("symbol")
        shares = int(request.form.get("shares"))

        # Hit the API to get symbol's price
        res = lookup(symbol)

        # Ensure the result is valid
        if res is None:
            return apology("Must enter a valid stock symbol")

        # Access the cash
        available_cash = getCash(session["user_id"])
        if not available_cash:
            return apology("Query failure")

        # Get symbol's current price
        price = float(res["price"])

        # Ensure the user has enough cash to perform the transaction
        if (price * shares) > available_cash:
            return apology("Not enough cash")

        # Write the transaction
        newTransaction(session["user_id"], symbol, shares, price, dt.datetime.now())

        # Substract cash from the user
        updateCash(session["user_id"], available_cash - (shares * price))

        # Inform the buy success
        flash("Bought!")

        return redirect("/")


@app.route("/history")
@login_required
def history():
    """Show history of transactions"""

    # Get the current user transactions
    transactions = getTransactions(session["user_id"])

    return render_template("history.html", rows=transactions)


@app.route("/login", methods=["GET", "POST"])
def login():
    """Log user in"""

    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":

        # Ensure username was submitted
        if not request.form.get("username"):
            return apology("must provide username", 403)

        # Ensure password was submitted
        elif not request.form.get("password"):
            return apology("must provide password", 403)

        # Query database for username
        rows = db.execute("SELECT * FROM users WHERE username = :username",
                          username=request.form.get("username"))

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(rows[0]["hash"], request.form.get("password")):
            return apology("invalid username and/or password", 403)

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    else:
        return render_template("login.html")


@app.route("/logout")
def logout():
    """Log user out"""

    # Forget any user_id
    session.clear()

    # Redirect user to login form
    return redirect("/")


@app.route("/quote", methods=["GET", "POST"])
@login_required
def quote():
    """Get stock quote."""

    if request.method == "GET":
        return render_template("quote.html")
    else:
        # Ensure the symbol is not blank
        if not request.form.get("symbol"):
            return apology("Must enter stock symbol")

        # Lookup for the symbol's current price and get a JSON object
        res = lookup(request.form.get("symbol"))

        # Ensure the symbol is valid
        if res is None:
            return apology("Invalid stock symbol")

        # Return the template to render and the API results
        return render_template("quoted.html", results=res)


@app.route("/new_password", methods = ["GET", "POST"])
@login_required
def newPassword():

    if request.method == "GET":
        return render_template("new_password.html")
    else:

        # Ensure password and confirmation aren't blank
        if not request.form.get("password"):
            return apology("Missing password")
        if not request.form.get("confirmation"):
            return apology("Missing password confirmation")

        # Get both fields
        password = request.form.get("password")
        confirmation = request.form.get("confirmation")

        # Ensure password length
        if len(password) < 4:
            return apology("Password at least 4 chars long!")

        # Ensure password and confirmation match
        if password != confirmation:
            return apology("Passwords must match!")

        #password = password_check(request.form.get("password"),
        #   request.form.get("confirmation"))

        # Update the 'users' table
        updatePassword(session["user_id"], password)

        return redirect("/login")

@app.route("/register", methods=["GET", "POST"])
def register():
    """Register user"""

    if request.method == "GET":
        return render_template("register.html")
    else:

        # Ensure username and password are not blank
        if not request.form.get("username"):
            return apology("Must provide a username")

        # Get the username
        username = request.form.get("username")

        # Ensure the password integrity
        value, msg = verify_password(request)

        if not value:
            return apology(msg)

        # Get the password
        password = request.form.get("password")

        # Register the new user in the database
        newUser(username, generate_password_hash(password))

        # Inform successfull register
        flash("Registered!")

        # Redirect to login
        return redirect("/login")

@app.route("/sell", methods=["GET", "POST"])
@login_required
def sell():
    """Sell shares of stock"""

    if request.method == "GET":

        # Get the user's symbols
        symbols = getSymbols(session["user_id"])

        return render_template("sell.html", symbols=symbols)
    else:

        # Ensure symbol and shares are not blank
        if not request.form.get("symbol"):
            return apology("Must select a symbol to sell")
        if not request.form.get("shares"):
            return apology("Must enter an amount of shares")

        # Get the symbol and amount of shares to sell
        symbol = request.form.get("symbol")
        shares = int(request.form.get("shares"))

        # Ensure the amount of shares is logically valid
        if shares < 0:
            return apology("Must enter a valid amount of shares")

        # Get user's amount of shares
        user_shares = getShares(session["user_id"], symbol)

        # Ensure the amount of shares to sell does not exceed the user's shares
        if shares > user_shares:
            return apology("Too much shares")

        # Get current price of the symbol
        res = lookup(symbol)
        if not res:
            return apology("Lookup error")

        # Write transaction
        newTransaction(session["user_id"], symbol, shares * (-1), res["price"], dt.datetime.now())

        # Get user's cash
        cash = getCash(session["user_id"])

        # Update user's cash
        updateCash(session["user_id"], cash + (shares * res["price"]))

        # Inform successfull sell
        flash("Sold!")

    return redirect("/")


def errorhandler(e):
    """Handle error"""
    if not isinstance(e, HTTPException):
        e = InternalServerError()
    return apology(e.name, e.code)


# Listen for errors
for code in default_exceptions:
    app.errorhandler(code)(errorhandler)

