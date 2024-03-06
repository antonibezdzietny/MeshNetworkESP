import mariadb

DATABASE_DATA = {
    "HOST": "127.0.0.1",
    "PORT": 3306,
    "DATABASE": "meshNetwork"
    }

def connect_database(db_user, db_password):
    attempts = 0

    while attempts < 3:
        try:
            conn = mariadb.connect(
                user=db_user,
                password=db_password,
                host=DATABASE_DATA["HOST"],
                port=DATABASE_DATA["PORT"],
                database=DATABASE_DATA["DATABASE"]
                )
            return conn
        except mariadb.Error as e:
            attempts += 1
            print(f"Error connecting to MariaDB Platform: {e}")
            time.sleep(1)
            if attempts >= 2:
                sys.exit(1)
