import type { HarrowSettings } from '../components/models';
import { api } from '../boot/axios';
import { defineStore } from 'pinia';

export function createSettingsState() {
  return {
    records: [] as HarrowSettings[],
  };
}

export const useSettingsStore = defineStore('SettingsStore', {
  state: () => {
    return createSettingsState();
  },
  getters: {
    settings: (state) => state.records,
  },
  actions: {
    async list() {
      // api.get('/list');
      const { data } = await api.get('list');
      console.log('list', data);
      // this.records = data as HarrowSettings[];
      return data;
    },
    async load(path: string) {
      const { data } = await api.get(`load/${path}`);
      return data;
    },
    async save(path: string, settings: HarrowSettings) {
      const { data } = await api.post(`save/${path}`, settings);
      return data;
    },
  },
});
