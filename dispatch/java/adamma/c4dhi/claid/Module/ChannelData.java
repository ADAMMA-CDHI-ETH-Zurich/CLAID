package adamma.c4dhi.claid.Module;

import java.time.LocalDateTime;

public class ChannelData<T>
{
    private final T data;
    private final LocalDateTime timestamp;
    private final String userToken;

    private final boolean valid;

    public ChannelData(final T data, final LocalDateTime timestamp, final String userToken)
    {
        this.data = data;
        this.timestamp = timestamp;
        this.userToken = userToken;
        this.valid = true;
    }

    public final T getData()
    {
        return this.data;
    }

    public final LocalDateTime getTimestamp()
    {
        return this.timestamp;
    }

    public final String getUserId()
    {
        return this.userToken;
    }

    public boolean isValid()
    {
        return this.valid;
    }

}
