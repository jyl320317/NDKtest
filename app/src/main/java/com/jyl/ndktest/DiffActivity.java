package com.jyl.ndktest;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.FileProvider;

import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;

import com.jyl.ndktest.databinding.ActivityDiffBinding;
import com.jyl.ndktest.util.ELog;

import java.io.File;

public class DiffActivity extends AppCompatActivity {

    static {
        System.loadLibrary("diff-lib");
    }

    private ActivityDiffBinding binding;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityDiffBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        binding.update.setOnClickListener(v -> update());

    }

    private void update() {
        String patch = getExternalFilesDir("test").getAbsolutePath() + "/patch";
        String output = getExternalFilesDir("test").getAbsolutePath() + "/new.apk";
        new AsyncTask<Void, Void, File>() {

            @Override
            protected File doInBackground(Void... voids) {
                String old = getApplication().getApplicationInfo().sourceDir;
                bspatch(old, patch, output);
                return new File(output);
            }

            @Override
            protected void onPostExecute(File file) {
                installApk(file);
            }
        }.execute();

    }

    private void installApk(File file) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
            intent.setDataAndType(Uri.fromFile(file), "application/vnd.android.package-archive");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        } else {
            // 声明需要的临时权限
            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            // 第二个参数，即第一步中配置的authorities
            String packageName = getApplication().getPackageName();
            Uri contentUri = FileProvider.getUriForFile(DiffActivity.this, packageName + ".provider", file);
            ELog.e(contentUri);
            intent.setDataAndType(contentUri, "application/vnd.android.package-archive");
        }
        startActivity(intent);
    }

    /**
     * @param oldapk 当前运行的apk
     * @param patch  差分包
     * @param output 合成后的新的apk输出到
     */
    public native void bspatch(String oldapk, String patch, String output);

}