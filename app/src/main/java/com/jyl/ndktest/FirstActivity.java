package com.jyl.ndktest;

import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;

import com.jyl.ndktest.databinding.ActivityFirstBinding;

import java.util.HashMap;
import java.util.Map;

public class FirstActivity extends AppCompatActivity {

    private final ActivityResultLauncher<String[]> launcher = registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), (ActivityResultCallback<Map<String, Boolean>>) result -> {

    });

    private ActivityFirstBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityFirstBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        launcher.launch(new String[]{
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
        });
        binding.play.setOnClickListener(v -> startActivity(new Intent(FirstActivity.this, MainActivity.class)));
        binding.diff.setOnClickListener(v -> startActivity(new Intent(FirstActivity.this, DiffActivity.class)));
    }

}