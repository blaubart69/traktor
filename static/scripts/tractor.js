angular.module('tractor', ['rzSlider'])
.controller('TractorController', function ($scope, $http) {
    // initialize the content type of post request with text/plaien
    // TO AVOID triggering the pre-flight OPTIONS request
    $http.defaults.headers.post["Content-Type"] = "text/plain";
    $scope.colorSlider = {
        minValue: 36,
        maxValue: 80,
        options: {
            floor: 0,
            ceil: 255
        }
    };
    $scope.greenSlider = {
        minValue: 80,
        maxValue: 255,
        options: {
            floor: 0,
            ceil: 255
        }
    };
    $scope.thresholdSlider = {
        value: 5,
        options: {
            floor: 2,
            ceil: 20
        }
    };
    $scope.maxMarkerSlider = {
      value: 10,
      options: {
          floor: 1,
          ceil: 50
      }
  }
  $scope.data = {
        duration: 1.0,
        left: 12,
        right: 13,
        direction: 'center',
        url: 'http://10.3.141.165:8888/video',
        colorFilter: true,
        detecting: false,
        colorFrom: '36,80,25',
        colorTo: '80,255,255',
        erode: 10,
        dilate: 10,
        contourMode: 'POLY',
        threshold: 5,
        maximumMarkers: 10
    };
    $scope.videoUrl = '/video?t=' + new Date().getTime();

    const handleResponse = function (response) {
        $scope.response = angular.toJson(response, true);
    };
    const applySliderValues = function () {
        $scope.data.colorFrom = $scope.colorSlider.minValue + ',' + $scope.greenSlider.minValue + ',25';
        $scope.data.colorTo = $scope.colorSlider.maxValue + ',' + $scope.greenSlider.maxValue + ',255';
        $scope.data.threshold = $scope.thresholdSlider.value;
        $scope.data.maximumMarkers = $scope.maxMarkerSlider.value;
    };
    this.stop = function () {
        this.applyChanges(false);
    };
    this.perform = function (direction) {
        $scope.data.direction = direction;
        $http.post('/perform', $scope.data).then(handleResponse);
    };
    this.applyChanges = function (detecting) {
        $scope.data.detecting = detecting;
        applySliderValues();
        $http.post('/applyChanges', $scope.data).then(handleResponse);
    };

    console.log("initialize controller");
});
