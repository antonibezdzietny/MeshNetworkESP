"""RESTApi.py - REST api"""
from flask import Flask
from flask import jsonify
from flask import json
from flask import request
from flask import Response
import sys
from DatabaseConfig import DATABASE_DATA, connect_database
import mariadb

app = Flask(__name__)

DATABASE_PASS = {
    "LOGIN": "UserSelector",
    "PASSWORD": "password1"
    }


def con_database():
    return connect_database(DATABASE_PASS["LOGIN"],
                            DATABASE_PASS["PASSWORD"])


def execute_select(sql, value=None):
    conn = con_database()
    cur = conn.cursor()
    if value == None:
        cur.execute(sql)
    else :
        cur.execute(sql, value)
    result = cur.fetchall()
    conn.close()
    return result
    
    
def prepareResponse(listToResponse):
    json_string = json.dumps(listToResponse)
    response = Response()
    response.data = json_string
    response.headers['content-length'] = str(len(json_string)-1)
    response.headers["Content-Type"] = "application/json; charset=utf-8"
    return response
    

#Return JSON table of sensors from database
@app.route('/get_sensors', methods=['POST'])
def get_sensors():
    sql = """SELECT SensorId, TopicName, ProperName, SensorType
                FROM Sensors INNER JOIN SensorsTypes ON
                Sensors.SensorTypeId = SensorsTypes.SensorTypeId"""
    result = execute_select(sql)

    sensors = [{"SensorId": SensorId,
                "TopicName": TopicName,
                "ProperName": StoreData,
                "SensorType": SensorType}
               for SensorId, TopicName, StoreData, SensorType in result]

    return prepareResponse(sensors)
    
    
#Return JSON table with measurement
@app.route('/get_values', methods=['POST'])
def get_values():
    request_data = request.get_json()
    
    sql = """SELECT DataTime, ROUND(Value,2) FROM Measurements WHERE
    SensorId IN (SELECT SensorId FROM Sensors WHERE TopicName = %(TopicName)s)
    AND DataTime BETWEEN %(TimeFrom)s AND %(TimeTo)s"""
    result = execute_select(sql, request_data)
    
    values = [{"DataTime": DataTime.strftime('%Y-%m-%d %H:%M:%S'),
               "Value": Value}
              for DataTime, Value in result]

    return prepareResponse(values)


#Return JSON statistic for measurement
@app.route('/get_statistics', methods=['POST'])
def get_statistics():
    request_data = request.get_json()

    sql = """SELECT COUNT(Value), ROUND(AVG(Value),2), ROUND(MIN(Value),2),
    ROUND(MAX(Value),2), ROUND(STD(Value),2) FROM Measurements WHERE
    SensorId IN (SELECT SensorId FROM Sensors WHERE TopicName = %(TopicName)s)
    AND DataTime BETWEEN %(TimeFrom)s AND %(TimeTo)s"""
    result = execute_select(sql, request_data)
    
    statistics = {"COUNT": result[0][0],
                    "AVG": result[0][1],
                    "MIN": result[0][2],
                    "MAX": result[0][3],
                    "STD": result[0][4]}
    
    return prepareResponse(statistics)

    
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=3456, debug=True)