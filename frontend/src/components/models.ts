export interface HarrowSettings {
  colorFrom: number[]
  colorTo: number[]

  erode: number
  dilate: number
  minimalContourArea: number

  maxRows: number
  rowSpacingPx: number
  rowPerspectivePx: number
  rowThresholdPx: number
  rowRangePx: number

  offsetPx: number
}
