package com.example.sensormeshnetwork;

import androidx.appcompat.app.AppCompatActivity;

import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.ListView;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class SensorSelectorActivity extends AppCompatActivity {
    final String SUB_PAGE = "get_sensors";

    ProgressDialog progressDialog;
    JSONObject [] jsonObjects;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor_selector);

        getAvailableSensorsFromDatabase();
    }


    private void getAvailableSensorsFromDatabase() {
        ExecutorService service = Executors.newSingleThreadExecutor();
        service.execute(new Runnable() {
            @Override
            public void run() {
                //onPreExecute Method
                runOnUiThread(() -> {
                    progressDialog = new ProgressDialog(SensorSelectorActivity.this);
                    progressDialog.setMessage(getString(R.string.progressDialogSensorSelector));
                    progressDialog.setIndeterminate(false);
                    progressDialog.setCancelable(false);
                    progressDialog.show();
                });

                //doInBackground Method
                String database_response = DatabaseConnector.createConnection(SUB_PAGE);
                if( database_response == null ){
                    jsonObjects = null;
                }else {
                    convertToJSONArrayObjects( database_response + "]" );
                }

                //onPostExecute Method
                runOnUiThread(() -> {
                    createListView();
                    progressDialog.cancel();
                });

            }
        });
    }


    private void convertToJSONArrayObjects(String s) {
        try {
            JSONArray jsonArray = new JSONArray(s);

            if( jsonArray.length() == 0 ){
                jsonObjects = null;
                return;
            }

            jsonObjects = new JSONObject[jsonArray.length()];
            for( int i=0; i<jsonArray.length(); ++i){
                jsonObjects[i] = jsonArray.getJSONObject(i);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

    }


    private void createListView() {
        if(jsonObjects == null){
            Toast.makeText( this,
                    getString(R.string.noAvailableSensors), Toast.LENGTH_LONG).show();
            return;
        }

        ListView listView = (ListView) findViewById(R.id.listview_sensor);
        SensorListAdapter sensorListAdapter = new SensorListAdapter(this, jsonObjects);
        listView.setAdapter(sensorListAdapter);
        listView.setOnItemClickListener((parent, view, position, id) -> handleOnClickItem(position));
    }


    private void handleOnClickItem(int position) {
        try {
            Intent intent = new Intent(this, GraphDisplayActivity.class);
            intent.putExtra("ProperName", jsonObjects[position].getString("ProperName"));
            intent.putExtra("TopicName", jsonObjects[position].getString("TopicName"));
            intent.putExtra("SensorType", jsonObjects[position].getString("SensorType"));
            startActivity(intent);
        } catch (JSONException e) {
            e.printStackTrace();
            Log.e("SensorSelectorActivity", "intent.putExtra()");
        }
    }
}