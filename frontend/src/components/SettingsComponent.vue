<template>
  <div class="q-px-lg q-pt-md q-pb-xl">
    <q-item>
      <q-item-section side>
        <q-select
          v-model="profileName"
          label="Profil"
          :options="profiles"
          use-input
          hide-selected
          fill-input
          input-debounce="0"
          dense
          filled
          @update:model-value="loadSettings"
        >
          <template v-slot:prepend>
            <q-icon name="file_open" />
          </template>
        </q-select>
      </q-item-section>
      <q-item-section side>
        <q-btn @click="save = true" dense>
          <q-icon name="save" />
        </q-btn>
      </q-item-section>
      <q-item-section side>
        <q-btn @click="disableFields" dense>
          <q-icon v-if="locked" name="lock_outline" />
          <q-icon v-if="!locked" name="lock_open" />
        </q-btn>
      </q-item-section>
    </q-item>
    <q-list bordered dense>
      <q-item>
        <q-item-section side>
          <q-icon name="blur_off">
            <q-tooltip>Erode</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.erode"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 10: '10' }"
            :min="0"
            :max="10"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="blur_on">
            <q-tooltip>Dilate</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.dilate"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 10: '10' }"
            :min="0"
            :max="10"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="crop">
            <q-tooltip>Minimal contour area</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.area"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 1000: '1000' }"
            :min="0"
            :max="1000"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="expand">
            <q-tooltip>Reihen Toleranz</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.rowThreshold"
            label-always
            switch-label-side
            :marker-labels="{ 1: '1', 320: '320' }"
            :min="1"
            :max="320"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="aspect_ratio">
            <q-tooltip>Reihen Bereich</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.rowRange"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 320: '320' }"
            :min="0"
            :max="320"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-separator spaced />
      <q-item>
        <q-item-section side>
          <q-icon name="palette">
            <q-tooltip>Farbe</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range
            v-model="vm.hue"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 179: '179' }"
            :min="0"
            :max="179"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="opacity">
            <q-tooltip>Sättigung</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range
            v-model="vm.satturation"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 255: '255' }"
            :min="0"
            :max="255"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="highlight">
            <q-tooltip>Helligkeit</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range
            v-model="vm.brightness"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 255: '255' }"
            :min="0"
            :max="255"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="vertical_shades">
            <q-tooltip>Anzahl der Reihen</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.maxRows"
            label-always
            switch-label-side
            :marker-labels="{ 0: '1', 7: '7' }"
            :min="1"
            :max="7"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="compress">
            <q-tooltip>Reihen Abstand</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.rowSpacing"
            label-always
            switch-label-side
            :marker-labels="{ 10: '10', 1000: '1000' }"
            :min="10"
            :max="1000"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="architecture">
            <q-tooltip>Reihen Perspektive</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-slider
            v-model="vm.rowPerspective"
            label-always
            switch-label-side
            :marker-labels="{ 0: '0', 1000: '1000' }"
            :min="0"
            :max="1000"
            :disable="locked"
          />
        </q-item-section>
      </q-item>
    </q-list>
    <q-dialog v-model="save" position="left">
      <q-card style="min-width: 350px">
        <q-bar>
          <q-icon name="save" />
          <div>Profil speichern</div>
          <q-space />
          <q-btn dense flat icon="close" v-close-popup>
            <q-tooltip>Close</q-tooltip>
          </q-btn>
        </q-bar>
        <q-card-section>
          <q-input v-model="profileName" label="Profil" clearable dense autofocus filled />
        </q-card-section>

        <q-card-actions align="right" class="text-primary">
          <q-btn flat label="Abbrechen" v-close-popup />
          <q-btn flat label="Speichern" @click="saveSettings" />
        </q-card-actions>
      </q-card>
    </q-dialog>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from 'vue';
import type { HarrowSettings, SettingsViewModel } from './models';
import { useSettingsStore } from '../stores/settings-store';
import { storeToRefs } from 'pinia';

const store = useSettingsStore();
const { profiles } = storeToRefs(store);
const vm = ref({} as SettingsViewModel);
const locked = ref(false);
const save = ref(false);
const profileName = ref('');

onMounted(async () => {
  await store.list();
  const current = await store.current();
  console.log('list', profiles.value);
  fromBackend(current);
});

watch(
  vm,
  async () => {
    await applyChanges();
  },
  { deep: true },
);

const disableFields = () => {
  locked.value = !locked.value;
};

const applyChanges = async () => {
  const data = toBackend();
  await store.applyChanges(data);
};

const saveSettings = async () => {
  const data = toBackend();
  await store.save(profileName.value, data);
  save.value = false;
};

const loadSettings = async (profile: string) => {
  const data = await store.load(profile);
  fromBackend(data);
};

const toBackend = () => {
  const data = {
    colorFrom: [
      vm.value.hue.min as number,
      vm.value.satturation.min as number,
      vm.value.brightness.min as number,
    ],
    colorTo: [
      vm.value.hue.max as number,
      vm.value.satturation.max as number,
      vm.value.brightness.max as number,
    ],
    dilate: vm.value.dilate,
    erode: vm.value.erode,
    minimalContourArea: vm.value.area,

    maxRows: vm.value.maxRows,
    rowRangePx: vm.value.rowRange,
    rowPerspectivePx: vm.value.rowPerspective,
    rowSpacingPx: vm.value.rowSpacing,
    rowThresholdPx: vm.value.rowThreshold,
  } as HarrowSettings;
  // TODO offset is missing
  return data;
};

const fromBackend = (data: HarrowSettings) => {
  vm.value = {
    hue: {
      min: data.colorFrom[0] as number,
      max: data.colorTo[0] as number,
    },
    satturation: {
      min: data.colorFrom[1] as number,
      max: data.colorTo[1] as number,
    },
    brightness: {
      min: data.colorFrom[2] as number,
      max: data.colorTo[2] as number,
    },

    dilate: data.dilate,
    erode: data.erode,
    area: data.minimalContourArea,

    maxRows: data.maxRows,
    rowRange: data.rowRangePx,
    rowPerspective: data.rowPerspectivePx,
    rowSpacing: data.rowSpacingPx,
    rowThreshold: data.rowThresholdPx,

    offset: data.offsetPx,
  };
};
</script>
