package adamma.c4dhi.claid.Module.Scheduling;

import java.time.LocalDate;
import java.time.LocalDateTime;
import java.util.function.Consumer;

public class ConsumerRunnable<T> extends ScheduledRunnable
{
    private T data;
    private Consumer<T> consumer;

    public ConsumerRunnable(T data, Consumer<T> consumer)
    {
        super(ScheduleOnce.now());
        this.consumer = consumer;
        this.data = data;
    }

    public ConsumerRunnable(T data, Consumer<T> consumer, ScheduleDescription schedule)
    {
        super(schedule);
        this.consumer = consumer;
        this.data = data;
    }

    @Override
    public void run() {
        consumer.accept(data);
    }    
}
