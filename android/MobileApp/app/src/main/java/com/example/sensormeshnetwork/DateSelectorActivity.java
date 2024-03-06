package com.example.sensormeshnetwork;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CalendarView;
import android.widget.TextView;
import android.widget.TimePicker;

import java.text.SimpleDateFormat;

public class DateSelectorActivity extends AppCompatActivity {
    TimePicker timePicker;
    CalendarView calendarView;
    Button button;

    String chooseDate;
    String chooseTime;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_date_selector);

        setTextHeaderFromBundle();

        assignWidgets();

        button.setOnClickListener(new View.OnClickListener() {
            @RequiresApi(api = Build.VERSION_CODES.M)
            @Override
            public void onClick(View v) {
                handleOnClick();
            }
        });


        chooseDate = new SimpleDateFormat("yyyy-MM-dd").format(calendarView.getDate());
        calendarView.setOnDateChangeListener(new CalendarView.OnDateChangeListener() {
            @Override
            public void onSelectedDayChange(CalendarView view, int year, int month, int dayOfMonth) {
                chooseDate = year + "-" + String.format("%02d",month+1) + "-" + String.format("%02d",dayOfMonth);
            }
        });

    }

    private void setTextHeaderFromBundle() {
        TextView textViewHeader = (TextView) findViewById(R.id.header_select_date);
        Bundle bundle = getIntent().getExtras();
        if (bundle == null) {
            textViewHeader.setText(getString(R.string.dateSelectorDefault));
            return;
        }
        textViewHeader.setText(getString(bundle.getInt("HeaderStringId")));
    }


    private void handleOnClick() {
        getCurrentTime();
        returnValue();
    }

    private void returnValue() {
        Intent output = new Intent();
        output.putExtra("DATE", chooseDate);
        output.putExtra("TIME", chooseTime);
        setResult(RESULT_OK, output);
        finish();
    }

    private void getCurrentTime() {
        int hour;
        int minute;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            hour= timePicker.getHour();
            minute = timePicker.getMinute();
        }else{
            hour = minute = 0;
        }

        chooseTime = String.format("%02d",hour) + ":" + String.format("%02d",minute) + ":" + "00";
    }

    private void assignWidgets() {
        button = (Button)findViewById(R.id.set_date_buton);
        calendarView = (CalendarView)findViewById(R.id.calendarView);
        timePicker = (TimePicker)findViewById(R.id.time_picker);
        timePicker.setIs24HourView(true);
    }
}