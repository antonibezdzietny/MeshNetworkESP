package com.example.sensormeshnetwork;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.text.format.Time;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;


import com.jjoe64.graphview.DefaultLabelFormatter;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class GraphDisplayActivity extends AppCompatActivity {
    final String SUB_PAGE_STATISTICS = "get_statistics";
    final String SUB_PAGE_VALUES     = "get_values";

    String properName;
    String topicName;
    String sensorType;

    //Widgets on GraphDisplayActivity
    TextView textViewTimeFrom;
    TextView textViewTimeTo;
    Button buttonSetTimeFrom;
    Button buttonSetTimeTo;
    Button buttonLoadData;
    ProgressDialog progressDialog;

    //Its variable for define onClick data type 0-NOTHING, 1-TIME_FROM, 2-TIME_TO
    int setTimeTypeButton = 0;

    //Measurements values & statistics
    JSONObject jsonObjectStatistics;
    JSONArray jsonArrayValues;
    //Series points
    LineGraphSeries<DataPoint> series;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_graph_display);

        getIntentParameters();
        setTextHeader();
        assignWidgets();
        setDefaultDateRange();
        setOnClickListenerForButtons();
        getDataFromDatabase();
    }

    private void getIntentParameters() {
        Bundle bundle = getIntent().getExtras();
        if (bundle == null)
            Log.e("GraphDisplayActivity",
                    "getIntentParameters() - No extra parameters!");

        properName = bundle.getString("ProperName");
        topicName = bundle.getString("TopicName");
        sensorType = bundle.getString("SensorType");
    }

    private void assignWidgets() {
        textViewTimeFrom = (TextView) findViewById(R.id.time_from);
        textViewTimeTo = (TextView) findViewById(R.id.time_to);
        buttonSetTimeFrom = (Button) findViewById(R.id.set_time_from);
        buttonSetTimeTo = (Button) findViewById(R.id.set_time_to);
        buttonLoadData = (Button) findViewById(R.id.load_data);
    }

    @SuppressLint("SetTextI18n")
    private void setDefaultDateRange() {
        Time currentTime = new Time();
        currentTime.setToNow();

        @SuppressLint("DefaultLocale")
        String data_day = currentTime.year + "-" +
                String.format("%02d", currentTime.month+1) + "-" +
                String.format("%02d", currentTime.monthDay);

        textViewTimeFrom.setText(data_day + " 00:00:00");
        textViewTimeTo.setText(data_day + " 23:59:59");
    }

    private void setTextHeader() {
        ImageView imageViewHeader = (ImageView) findViewById(R.id.icon_type);
        imageViewHeader.setImageResource(SensorType.getIconId(this,
                SensorType.getType(sensorType)));

        TextView textViewProperName = (TextView) findViewById(R.id.proper_name);
        textViewProperName.setText(properName);

        TextView textViewTopicName = (TextView) findViewById(R.id.topic_name);
        textViewTopicName.setText(topicName);
    }

    private void setOnClickListenerForButtons(){
        buttonLoadData.setOnClickListener(v -> {
            getDataFromDatabase();
        });

        buttonSetTimeFrom.setOnClickListener(v -> {
            setTimeTypeButton = 1;
            launchDataSelector(R.string.dateSelectorFrom);
        });

        buttonSetTimeTo.setOnClickListener(v -> {
            setTimeTypeButton = 2;
            launchDataSelector(R.string.dateSelectorTo);
        });
    }

    private void launchDataSelector(int string_id){
        Intent intent = new Intent(this, DateSelectorActivity.class);
        intent.putExtra("HeaderStringId", string_id);
        launcher.launch(intent);
    }

    ActivityResultLauncher<Intent> launcher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), new ActivityResultCallback<ActivityResult>() {
        @Override
        public void onActivityResult(ActivityResult result) {
            if(result.getResultCode() == Activity.RESULT_OK && result.getData() != null){
                handleReturnValueDataSelector(result.getData());
            }
        }
    });

    private void handleReturnValueDataSelector(Intent data) {
        String date = data.getStringExtra("DATE") + " " + data.getStringExtra("TIME");
        if(setTimeTypeButton == 1)
            textViewTimeFrom.setText(date);
        else if (setTimeTypeButton == 2)
            textViewTimeTo.setText(date);

        setTimeTypeButton = 0;
    }


    private void getDataFromDatabase() {
        ExecutorService service = Executors.newSingleThreadExecutor();
        service.execute(new Runnable() {
            @Override
            public void run() {
                //onPreExecute Method
                runOnUiThread(() -> {
                    progressDialog = new ProgressDialog(GraphDisplayActivity.this);
                    progressDialog.setMessage(getString(R.string.progressDialogGraphDisplay));
                    progressDialog.setIndeterminate(false);
                    progressDialog.setCancelable(false);
                    progressDialog.show();
                });

                //doInBackground Method
                    readStatisticsFromDatabase();
                    try {
                        if(jsonObjectStatistics.getInt("COUNT") > 0){
                            showStatistics();
                            readValuesFromDatabase();
                            getValueForPlot();
                            displayPlot();
                        }
                    }catch (JSONException e) {
                        e.printStackTrace();
                    }


                //onPostExecute Method
                runOnUiThread(() -> {
                    progressDialog.cancel();
                });

            }
        });
    }

    private void readStatisticsFromDatabase() {
        String response_statistics = DatabaseConnector.createConnection(
                SUB_PAGE_STATISTICS, createJSONRequest());
        try {
            jsonObjectStatistics = new JSONObject(response_statistics+"}");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void readValuesFromDatabase() {
        String response_values = DatabaseConnector.createConnection(
                SUB_PAGE_VALUES, createJSONRequest());

        try {
            jsonArrayValues = new JSONArray(response_values+"]");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private JSONObject createJSONRequest(){
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("TopicName", topicName);
            jsonObject.put("TimeFrom", textViewTimeFrom.getText());
            jsonObject.put("TimeTo", textViewTimeTo.getText());
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return jsonObject;
    }

    private void showStatistics() {
        TextView textViewAvg = (TextView) findViewById(R.id.average_value);
        TextView textViewStd = (TextView) findViewById(R.id.standar_deviation);
        TextView textViewMin = (TextView) findViewById(R.id.minimal_value);
        TextView textViewMax = (TextView) findViewById(R.id.maximal_value);
        TextView textViewCount = (TextView) findViewById(R.id.count_value);

        try {
            textViewAvg.setText(String.valueOf(jsonObjectStatistics.getDouble("AVG")));
            textViewStd.setText(String.valueOf(jsonObjectStatistics.getDouble("STD")));
            textViewMin.setText(String.valueOf(jsonObjectStatistics.getDouble("MIN")));
            textViewMax.setText(String.valueOf(jsonObjectStatistics.getDouble("MAX")));
            textViewCount.setText(String.valueOf(jsonObjectStatistics.getInt("COUNT")));
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void getValueForPlot() {
        series = null;
        series = new LineGraphSeries<>();
        if(jsonArrayValues.length() < 2)
            return;

        @SuppressLint("SimpleDateFormat") SimpleDateFormat dateFormat =
                new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        JSONObject jsonObject = new JSONObject();
        for(int i=0; i<jsonArrayValues.length(); ++i){
            try {
                jsonObject = jsonArrayValues.getJSONObject(i);
                series.appendData( new DataPoint(Objects.requireNonNull(dateFormat.parse(jsonObject.getString("DataTime"))),
                        jsonObject.getDouble("Value")),true, jsonArrayValues.length() );
            } catch (ParseException | JSONException e) {
                e.printStackTrace();
            }
        }
    }

    private void displayPlot() {
        @SuppressLint("SimpleDateFormat") SimpleDateFormat dateFormat =
                new SimpleDateFormat("\nyyyy-MM-dd\nHH:mm:ss");
        @SuppressLint("SimpleDateFormat") SimpleDateFormat dateFormat2 =
                new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        GraphView graphView = (GraphView) findViewById(R.id.graph);
        graphView.removeAllSeries();
        graphView.addSeries(series);

        graphView.getGridLabelRenderer().setLabelFormatter(new DefaultLabelFormatter(){
            @Override
            public String formatLabel(double value, boolean isValueX) {
                if(isValueX){
                    return dateFormat.format(new Date((long)value));
                }
                return super.formatLabel(value, isValueX);
            }
        });

        graphView.getViewport().setMaxX(series.getHighestValueX());
        graphView.getViewport().setMinX(series.getLowestValueX());
        graphView.getViewport().setXAxisBoundsManual(true);

        graphView.getViewport().setMaxY(series.getHighestValueY()+1);
        graphView.getViewport().setMinY(series.getLowestValueY()-1);
        graphView.getViewport().setXAxisBoundsManual(true);

        graphView.getGridLabelRenderer().setLabelHorizontalHeight(250);
        graphView.getGridLabelRenderer().setNumHorizontalLabels(5);
        graphView.getGridLabelRenderer().setHorizontalLabelsAngle(45);
        graphView.getGridLabelRenderer().reloadStyles();
    }


}