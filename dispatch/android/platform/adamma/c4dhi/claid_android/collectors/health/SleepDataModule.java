package adamma.c4dhi.claid_android.collectors.health;
import android.content.Context;
import androidx.health.connect.client.HealthConnectClient;
import androidx.health.connect.client.records.SleepSessionRecord;

import androidx.health.connect.client.records.metadata.DataOrigin;
import androidx.health.connect.client.request.ReadRecordsRequest;
import androidx.health.connect.client.response.ReadRecordsResponse;
import androidx.health.connect.client.time.TimeRangeFilter;
import androidx.health.platform.client.proto.Duration;

import java.time.Instant;
import java.time.ZonedDateTime;
import java.time.ZoneId;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid_sensor_data.SleepData;
import adamma.c4dhi.claid_sensor_data.SleepStage;
import adamma.c4dhi.claid_sensor_data.SleepStageType;

import kotlin.coroutines.EmptyCoroutineContext;
import kotlin.reflect.KClass;
import kotlinx.coroutines.BuildersKt;

public class SleepDataModule extends Module {

    private Channel<SleepData> sleepChannel;
    private Context context;

    public SleepDataModule(Context context) {
        this.context = context;
    }

    @Override
    protected void initialize(Properties properties) {
        sleepChannel = publish("SleepDataOutput", SleepData.class);
    }

    void gatherDataOfToday() {
        HealthConnectClient healthConnectClient = HealthConnectClient.getOrCreate(context);

        Instant endTime = Instant.now();
        Instant startTime = endTime.minus(24, ChronoUnit.HOURS);

        TimeRangeFilter timeRangeFilter = TimeRangeFilter.between(startTime, endTime);
        HashSet<DataOrigin> dor = new HashSet<>();
        boolean ascending = true;
        int maxRecords = 2000;
        ReadRecordsRequest request =
                new ReadRecordsRequest(
                        kotlin.jvm.JvmClassMappingKt.getKotlinClass(SleepSessionRecord.class),
                        timeRangeFilter,
                        dor,
                        ascending,
                        maxRecords,
                        null);

        ReadRecordsResponse response = null;
        try {
            response = BuildersKt.runBlocking(
                    EmptyCoroutineContext.INSTANCE,
                    (s, c) -> healthConnectClient.readRecords(request, c)
            );
        } catch (InterruptedException e) {
            Logger.logError("Reading sleep records failed: " + e.getMessage());
            return;
        }

        SleepData.Builder sleepData = SleepData.newBuilder();
        sleepData.setBeginOfSleepDataIntervalUnixTimestampMs(startTime.toEpochMilli());
        sleepData.setEndOfSleepDataIntervalUnixTimestampMs(endTime.toEpochMilli());

        for (Object datapointObj : response.getRecords()) {
            if (datapointObj instanceof androidx.health.connect.client.records.Record) {
                if (datapointObj instanceof androidx.health.connect.client.records.Record) {
                    androidx.health.connect.client.records.Record datapoint = (androidx.health.connect.client.records.Record) datapointObj;
                    // DATA_TYPES here we need to add support for each different data type
                    if (datapoint instanceof SleepSessionRecord) {
                        SleepSessionRecord sleepSessionRecord = (SleepSessionRecord) datapoint;


                        for(SleepSessionRecord.Stage stage : sleepSessionRecord.getStages())
                        {
                            SleepStage.Builder sleepStage = SleepStage.newBuilder();
                            sleepStage.setStartTimeUnixTimestamp(stage.getStartTime().toEpochMilli());
                            sleepStage.setEndTimeUnixTimestamp(stage.getEndTime().toEpochMilli());
                            SleepStageType.forNumber(stage.getStage());
                        }
                    }
                }
            }
        }


        sleepChannel.post(sleepData.build());
    }


}

