package com.example.sensormeshnetwork;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        startActivity(new Intent(this, SensorSelectorActivity.class));
    }

    @Override
    protected void onRestart() {
        startActivity(new Intent(this, SensorSelectorActivity.class));
        super.onRestart();
    }
}