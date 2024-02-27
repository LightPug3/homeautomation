"""
Flask Documentation:     https://flask.palletsprojects.com/
Jinja2 Documentation:    https://jinja.palletsprojects.com/
Werkzeug Documentation:  https://werkzeug.palletsprojects.com/
This file creates your application.
"""

# from crypt import methods
import site 

from app import app, Config,  mongo, Mqtt
from flask import escape, render_template, request, jsonify, send_file, redirect, make_response, send_from_directory 
from json import dumps, loads 
from werkzeug.utils import secure_filename
from datetime import datetime,timedelta, timezone
from os import getcwd
from os.path import join, exists
from time import time, ctime
from math import floor
 



#####################################
#   Routing for your application    #
#####################################


# 1. CREATE ROUTE FOR '/api/set/combination'
@app.route('/api/set/combination/', methods=['POST'])
def set_combination():
    if request.method == 'POST':
        try:
            # Extract passcode from form data
            form = request.form
            passcode = escape(form.get("passcode"))
            print(passcode)
            passcodeInt = int(passcode)
            passcode = str(passcode)
            # Check if passcode is a 4-digit integer
            if  len(passcode) == 4 and type(passcodeInt) == int :
                # Update passcode in the database
                success = mongo.update_passcode(passcode)   
            if success:
                return jsonify({"status": "complete", "data": "complete"})
            else:
                return jsonify({"status": "failed", "data": "failed"})
        except Exception as e:
            print(f"set_combination error: f{str(e)}")

# 2. CREATE ROUTE FOR '/api/check/combination'
@app.route('/api/check/combination/', methods=['POST'])
def check_combination():
    print("here")
    if request.method == 'POST':
        try:
            form =  request.form
 
            passcode = escape(form.get("passcode"))
            print(passcode)
            # Call the function to check the passcode
            print(passcode)
            result = mongo.check_passcode(passcode)
            if result:
                return jsonify({"status": "complete", "data": "complete"})
            else:
                return jsonify({"status": "failed", "data": "failed"})
        except Exception as e:
            print(f"check_combination error: f{str(e)}")

# 3. CREATE ROUTE FOR '/api/update'
@app.route('/api/update', methods=['POST'])
def update_data():
    print("here")
    if request.method == 'POST'and request.is_json:
        try:
            # Adding timestamp to the received data
            #data['timestamp'] = int(time.time())
            # Publish the modified data to a topic subscribed to by the frontend
            json_data = request.get_json()
            #print(json_data)
            # Add timestamp to the received data
            now = datetime.now()
            timestamp = int(now.timestamp())
            json_data['timestamp'] = timestamp
            Mqtt.publish('620156694_sub', json.dumps(json_data))
            Mqtt.publish('620156694_pub', json.dumps(json_data))
            Mqtt.publish('620156694', json.dumps(json_data))

            # Insert the modified object into the 'radar' collection of the database
            result = mongo.insert_data(json_data)
            if result:
                return jsonify({"status": "complete", "data": "complete"})
            else:
                return jsonify({"status": "failed", "data": "failed"})
        except Exception as e:
           print(f"update_data error: f{str(e)}")

# 4. CREATE ROUTE FOR '/api/reserve/<start>/<end>'
@app.route('/api/reserve/<start>/<end>', methods=['GET'])
def get_reservation(start, end):
     start = int(start)
     end = int(end)
    # Call function to get reserved objects
     if request.method == 'GET':
        try:
            data = mongo.get_reserved_objects(start, end)
            if data:
                return jsonify({"status": "found", "data": data})
        except Exception as e:
           print(f"get_reservation error: f{str(e)}")
        return jsonify({"status": "failed", "data": 0})

# 5. CREATE ROUTE FOR '/api/avg/<start>/<end>'
@app.route('/api/avg/<start>/<end>', methods=['GET'])
def get_average(start, end):
    # Call function to calculate average
     if request.method == 'GET':
        try:
            average = mongo.get_average(start, end)
            if average:
                return jsonify({"status": "found", "data": average})
        except Exception as e:
            return jsonify({"status": "failed", "data": 0})





@app.route('/api/file/get/<filename>', methods=['GET']) 
def get_images(filename):   
    '''Returns requested file from uploads folder'''
   
    if request.method == "GET":
        directory   = join( getcwd(), Config.UPLOADS_FOLDER) 
        filePath    = join( getcwd(), Config.UPLOADS_FOLDER, filename) 

        # RETURN FILE IF IT EXISTS IN FOLDER
        if exists(filePath):        
            return send_from_directory(directory, filename)
        
        # FILE DOES NOT EXIST
        return jsonify({"status":"file not found"}), 404


@app.route('/api/file/upload',methods=["POST"])  
def upload():
    '''Saves a file to the uploads folder'''
    
    if request.method == "POST": 
        file     = request.files['file']
        filename = secure_filename(file.filename)
        file.save(join(getcwd(),Config.UPLOADS_FOLDER , filename))
        return jsonify({"status":"File upload successful", "filename":f"{filename}" })

 


###############################################################
# The functions below should be applicable to all Flask apps. #
###############################################################


@app.after_request
def add_header(response):
    """
    Add headers to both force latest IE rendering engine or Chrome Frame,
    and also tell the browser not to cache the rendered page. If we wanted
    to we could change max-age to 600 seconds which would be 10 minutes.
    """
    response.headers['X-UA-Compatible'] = 'IE=Edge,chrome=1'
    response.headers['Cache-Control'] = 'public, max-age=0'
    return response

@app.errorhandler(405)
def page_not_found(error):
    """Custom 404 page."""    
    return jsonify({"status": 404}), 404



