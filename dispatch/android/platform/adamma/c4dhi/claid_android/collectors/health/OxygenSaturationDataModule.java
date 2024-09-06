package adamma.c4dhi.claid_android.collectors.health;
import android.content.Context;
import androidx.health.connect.client.HealthConnectClient;
import androidx.health.connect.client.records.OxygenSaturationRecord;
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
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid_sensor_data.SleepData;
import adamma.c4dhi.claid_sensor_data.SleepStage;
import adamma.c4dhi.claid_sensor_data.SleepStageType;
import adamma.c4dhi.claid_sensor_data.OxygenSaturationData;
import adamma.c4dhi.claid_sensor_data.OxygenSaturationSample;

import kotlin.coroutines.EmptyCoroutineContext;
import kotlin.reflect.KClass;
import kotlinx.coroutines.BuildersKt;

public class OxygenSaturationDataModule extends Module {

    private Channel<OxygenSaturationData> oxygenSaturationDataChannel;

    @Override
    protected void initialize(Properties properties) {
        oxygenSaturationDataChannel = publish("OxygenSaturationData", OxygenSaturationData.class);
    }

    void gatherDataOfToday() {
        HealthConnectClient healthConnectClient = HealthConnectClient.getOrCreate(CLAID.getContext());

        Instant endTime = Instant.now();
        Instant startTime = endTime.minus(24, ChronoUnit.HOURS);

        TimeRangeFilter timeRangeFilter = TimeRangeFilter.between(startTime, endTime);
        HashSet<DataOrigin> dor = new HashSet<>();
        boolean ascending = true;
        int maxRecords = 2000;
        ReadRecordsRequest request =
                new ReadRecordsRequest(
                        kotlin.jvm.JvmClassMappingKt.getKotlinClass(OxygenSaturationRecord.class),
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

        OxygenSaturationData.Builder oxygenSaturationData = OxygenSaturationData.newBuilder();
      //  oxygenSaturationData.setBeginOfOxygenDataIntervalUnixTimestampMs(startTime.toEpochMilli());
     //   oxygenSaturationData.setEndOfOxygenDataIntervalUnixTimestampMs(endTime.toEpochMilli());

        for (Object datapointObj : response.getRecords()) {
            if (datapointObj instanceof androidx.health.connect.client.records.Record) {
                androidx.health.connect.client.records.Record datapoint = (androidx.health.connect.client.records.Record) datapointObj;
                // DATA_TYPES here we need to add support for each different data type
                if (datapoint instanceof OxygenSaturationRecord) {
                    OxygenSaturationRecord oxygenSaturationRecord = (OxygenSaturationRecord) datapoint;

                    OxygenSaturationSample.Builder oxygenSaturationSample = OxygenSaturationSample.newBuilder();
                    oxygenSaturationSample.setUnixTimestampMs(oxygenSaturationRecord.getTime().toEpochMilli());
                    oxygenSaturationSample.setOxygenSaturationPercentage(oxygenSaturationRecord.getPercentage().getValue());

                    oxygenSaturationData.addOxygenSaturationSamples(oxygenSaturationSample.build());
                }
            }
        }


        oxygenSaturationDataChannel.post(oxygenSaturationData.build());
    }


}

