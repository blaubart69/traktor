package bumsti;

import java.util.Optional;

public interface IBaselineCalcOptional {
    Optional<Integer> DeltaToNearestRefLine(final int x_screen, final int y_screen, final CalcSettings settings);
}
