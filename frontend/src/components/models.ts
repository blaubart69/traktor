export interface HarrowSettings {
  colorFrom: number[];
  colorTo: number[];

  erode: number;
  dilate: number;
  minimalContourArea: number;

  maxRows: number;
  rowSpacingPx: number;
  rowPerspectivePx: number;
  rowThresholdPx: number;
  rowRangePx: number;

  offsetPx: number;
}

export interface MinMaxModel {
  min: number;
  max: number;
}

export interface SettingsViewModel {
  hue: MinMaxModel;
  satturation: MinMaxModel;
  brightness: MinMaxModel;

  erode: number;
  dilate: number;
  area: number;

  maxRows: number;
  rowSpacing: number;
  rowPerspective: number;
  rowThreshold: number;
  rowRange: number;

  offset: number;
}
