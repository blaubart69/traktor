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
          <q-range v-model="model" label-always :min="0" :max="179" />
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
          <q-range v-model="model" label-always :min="0" :max="179" />
        </q-item-section>
      </q-item>
      <q-item>
        <q-item-section side>
          <q-icon name="highlight">
            <q-tooltip>Helligkeit</q-tooltip>
          </q-icon>
        </q-item-section>
        <q-item-section>
          <q-range v-model="model" label-always :min="0" :max="179" />
        </q-item-section>
      </q-item>
    </q-list>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
// import type { HarrowSettings } from './models'

const model = ref({
  min: 36,
  max: 80,
})
</script>
