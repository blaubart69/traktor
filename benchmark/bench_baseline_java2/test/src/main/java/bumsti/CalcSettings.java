package bumsti;

public class CalcSettings {
    final public int x_half;
    final public int rowPerspectivePx;
    final public int y_baseline;
    final public int offset;
    final public int refline_distance;
    final public int half_refline_distance;
    final public int range_baseline;

    public CalcSettings(int x_half, int y_screen_size, int rowPerspectivePx, int refline_distance, int offset, int row_count) {
        this.x_half =  x_half;
       this.rowPerspectivePx=rowPerspectivePx;
       this.y_baseline = rowPerspectivePx + y_screen_size;
       this.offset = offset;
       this.refline_distance = refline_distance;
       this.half_refline_distance = (refline_distance / 2);
       this.range_baseline = ( ( refline_distance * row_count ) + (refline_distance / 2) );
    }
}
