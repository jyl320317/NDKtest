package com.jyl.ndktest;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.Configuration;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.Toast;

import com.jyl.ndktest.bean.Person;
import com.jyl.ndktest.player.Player;
import com.jyl.ndktest.util.ELog;

/**
 * 主页
 * <p></p>
 *
 * @author yonglin
 */
public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    static {
        System.loadLibrary("native-lib");
    }

    private Player player;

    private SeekBar seekBar;

    private int progress;
    private boolean isTouch;
    private boolean isSeek;
    private final String url = "/storage/emulated/0/DCIM/Camera/VID_20210207_151401.mp4";
//    private final String url = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceview);
        seekBar = findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        player = new Player();
        player.setSurfaceView(surfaceView);
        player.setDataSource(url);
        player.setPrepareListener(() -> {
            //获得时间
            int duration = player.getDuration();
            //直播： 时间就是0
            if (duration != 0) {
                runOnUiThread(() -> {
                    //显示进度条
                    seekBar.setVisibility(View.VISIBLE);
                    Toast.makeText(MainActivity.this, "可以开始播放了", Toast.LENGTH_LONG).show();
                });
            }
            player.start();
        });
        player.setOnProgressListener(progress -> {
            if (!isTouch) {
                runOnUiThread(() -> {
                    int duration = player.getDuration();
                    //如果是直播
                    if (duration != 0) {
                        if (isSeek) {
                            isSeek = false;
                            return;
                        }
                        //更新进度 计算比例
                        seekBar.setProgress(progress * 100 / duration);
                    }
                });
            }
        });
        player.setOnErrorListener(error -> Toast.makeText(MainActivity.this, "播放出错 代号=" + error, Toast.LENGTH_LONG).show());

//        test1(true, (byte) 1, 'c', (short) 3, 3, 3.3f, 4.0, "bob", 28,
//                new int[]{1, 2, 3, 4, 5},
//                new String[]{"bob", "jack"}, new Person(28, "jack"), new boolean[]{true, false});
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager
                    .LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceview);
        seekBar = findViewById(R.id.seekBar);
        player.setSurfaceView(surfaceView);
        player.setDataSource(url);
        seekBar.setOnSeekBarChangeListener(this);
        seekBar.setProgress(progress);
    }

    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }

    @Override
    protected void onPause() {
        super.onPause();
        ELog.e("onPause");
        player.stop();
    }

    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isSeek = true;
        isTouch = false;
        progress = player.getDuration() * seekBar.getProgress() / 100;
        //进度调整
        player.seek(progress);
    }


    public native String stringFromJni();

    /**
     * Java 将数据传递到 native 中
     */
    public native void test1(
            boolean b,
            byte b1,
            char c,
            short s,
            long l,
            float f,
            double d,
            String name,
            int age,
            int[] i,
            String[] strs,
            Person person,
            boolean[] bArray
    );

}