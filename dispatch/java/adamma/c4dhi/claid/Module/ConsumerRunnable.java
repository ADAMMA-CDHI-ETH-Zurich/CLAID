package adamma.c4dhi.claid.Module;

import java.util.function.Consumer;

public class ConsumerRunnable<T> implements Runnable
{
    private T data;
    private Consumer<T> consumer;

    ConsumerRunnable(T data, Consumer<T> consumer)
    {
        this.consumer = consumer;
        this.data = data;
    }

    @Override
    public void run() {
        consumer.accept(data);
    }    
}
