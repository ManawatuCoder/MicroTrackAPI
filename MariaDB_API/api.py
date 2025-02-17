import sqlalchemy, datetime
from sqlalchemy.ext.declarative import declarative_base
from flask import Flask, request, jsonify

app = Flask(__name__)

#Following line uses credentials from your MariaDB, which you should have set up.
engine = sqlalchemy.create_engine("mariadb+mariadbconnector://[username]:[password}@[IP - 127.0.0.1 for same device as DB]:[PORT]/light_levels")

Base = sqlalchemy.orm.declarative_base()

class Entries(Base):
        __tablename__ = 'light_levels'
        date_time = sqlalchemy.Column(sqlalchemy.String(50), primary_key=True)
        light_level = sqlalchemy.Column(sqlalchemy.Integer)
        device_id = sqlalchemy.Column(sqlalchemy.Integer)#Can change to any type desired, as long as DB reflects these changes.

Base.metadata.create_all(engine)

Session = sqlalchemy.orm.sessionmaker()
Session.configure(bind=engine)
session = Session()

@app.route('/add_value', methods=['POST'])
def add_value():
        data = request.get_data(as_text = True) #Receives sent parameters.
        if not data:
                print("No Value Provided")
                return "No Value Provided", 400
        try:
                date_time, light_level, device_id = data.split(",") #Separates string into useable variables
                print(data + " received")
                newLightLevel = Entries(date_time=date_time, light_level=light_level, device_id=device_id) #Populates columns
                session.add(newLightLevel) #Sends to DB
                session.commit() #Required for DB to enforce changes.
                return "message received", 200
        except Exception as error:
                print("Invalid Value Provided")
                print(error)
                session.rollback()
                return "Invalid Value Provided", 400

@app.route('/get_time', methods=['GET'])
def get_time():
        current_time = datetime.datetime.now()
        current_time = current_time.strftime("%Y %m %d %H %M %S")
        return current_time

if __name__ == '__main__':
        app.run(host='0.0.0.0', port=5000) #0.0.0.0 listens for all devices on network. Port is mostly arbitrary.