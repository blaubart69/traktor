package bumsti;

import org.openjdk.jmh.annotations.CompilerControl;

public class BaselineSimple {
    
    //
    // 0,0 ... fluchtpunkt
    //
    private static CoordPoint project_screen_point_to_coord(final int x_point_screen, final int y_point_screen, final int x_half, final int rowPerspectivePx) 
    {
        return
            new CoordPoint(
                 x_point_screen   - x_half
                ,rowPerspectivePx + y_point_screen);
    }
    //
    //  Fluchtpunkt: y=0
    //  baseline:    line on the lower end of the screen
    //  y_baseline:  vertical distance from Fluchtpunkt to lower end of the screen.
    //               screen height + y_fluchtpunkt
    //
    private static Integer project_x_onto_baseline(final CoordPoint point, final int y_baseline) //, int *x_base_line) 
    {
        if ( point.x == 0 ) 
        {
            // point is on the middle line
            // projected to the baseline: x = 0
            //*x_base_line = 0;
            return 0;
        }
        else if ( point.y == 0) 
        {
            // point is on the same y as the Fluchtpunkt
            // projecting to the baseline is not possible.
            // Hoff ma's...
            // ToDo: wos is mit point.y < 0???
            //   ein Punkt hinter dem Fluchtpunkt.
            //   Kaun NIE sei...
            return null;
        }
        else
        {
            final float steigung = (float)point.y / (float)point.x;
            int x_base_line = (int) ( (float)y_baseline / steigung );
            return x_base_line;
        }
    }

    private static int apply_offset(int x, int offset)
    {
        int x_offset =  x - offset;
        return x_offset;
    }

    private static boolean is_within_range(int x, final int range_baseline)
    {
        return Math.abs(x) < range_baseline;
        //return (unsigned int)std::abs(x) < range_baseline;
    }

    private static int project_x_inbetween_first_row(final int x_baseline, final int refline_distance) 
    {
        if ( refline_distance == 0)
        {
            // 2024-02-10 Spindler
            // don't know if this makes sense.
            // refline_distance == 0 means there are no reflines?
            return x_baseline;
        }
        else
        {
            return x_baseline % refline_distance;
        }
    }

    private static int distance_to_nearest_refline_on_baseline(final int x_first_row, final int refline_distance, final int half_refline_distance)
    {
        if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
        if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
        return x_first_row;
    }

    //@CompilerControl(CompilerControl.Mode.DONT_INLINE)
    public static Integer calc_baseline_delta_from_nearest_refline_simple(final int x_screen, final int y_screen, final CalcSettings settings)
    {
        CoordPoint p;
        p = project_screen_point_to_coord(x_screen,y_screen, settings.x_half, settings.rowPerspectivePx);

        Integer x_baseline = project_x_onto_baseline(p, settings.y_baseline);
        if ( x_baseline == null)
        {
            return null;
        }

        final int x_baseline_offset = apply_offset(x_baseline.intValue(), settings.offset);

        if ( ! is_within_range(x_baseline_offset, settings.range_baseline) )
        {
            return null;
        }

        final int x_baseline_first_row = project_x_inbetween_first_row(x_baseline_offset, settings.refline_distance);

        int delta_pixels = distance_to_nearest_refline_on_baseline(x_baseline_first_row, settings.refline_distance, settings.half_refline_distance);

        return delta_pixels;
    }
}
