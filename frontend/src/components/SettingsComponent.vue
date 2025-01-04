<template>
  <div class="q-px-lg q-pt-md q-pb-xl">
    <q-list bordered>
      <q-item>
        <q-item-section side>
          <q-icon name="palette">
            <q-tooltip>Farbe</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range v-model="hue" label-always :min="0" :max="179" />
        </q-item-section>
      </q-item>
      <q-separator spaced />
      <q-item>
        <q-item-section side>
          <q-icon name="opacity">
            <q-tooltip>Sättigung</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range v-model="satturation" label-always :min="0" :max="255" />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="highlight">
            <q-tooltip>Helligkeit</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range v-model="brightness" label-always :min="0" :max="255" />
        </q-item-section>
      </q-item>
    </q-list>
    <q-item>
      <q-item-section side>
        <q-btn @click="applyChanges"><q-icon name="done" /></q-btn>
      </q-item-section>
    </q-item>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import type { HarrowSettings } from './models';
import { onMounted } from 'vue';
import { useSettingsStore } from '../stores/settings-store';

const store = useSettingsStore();

const hue = ref({
  min: 36,
  max: 80,
});
const satturation = ref({
  min: 0,
  max: 180,
});
const brightness = ref({
  min: 0,
  max: 180,
});

onMounted(async () => {
  await store.list();
});

const erode = ref(3);
const dilate = ref(3);
const area = ref(130);

const maxRows = ref(3);
const rowRangePx = ref(160);
const rowPerspectivePx = ref(200);
const rowSpacingPx = ref(5);
const rowThresholdPx = ref(5);

const applyChanges = () => {
  const data = {} as HarrowSettings;

  /*
  data.colorFrom[0] = hue.value.min
  data.colorTo[0] = hue.value.max
  data.colorFrom[1] = satturation.value.min
  data.colorTo[1] = satturation.value.max
  data.colorFrom[2] = brightness.value.min
  data.colorTo[2] = brightness.value.max
  */

  data.dilate = dilate.value;
  data.erode = erode.value;
  data.minimalContourArea = area.value;

  data.maxRows = maxRows.value;
  data.rowRangePx = rowRangePx.value;
  data.rowPerspectivePx = rowPerspectivePx.value;
  data.rowSpacingPx = rowSpacingPx.value;
  data.rowThresholdPx = rowThresholdPx.value;

  console.log(data);
  store.save('lastSettings.json', data);
};
</script>
