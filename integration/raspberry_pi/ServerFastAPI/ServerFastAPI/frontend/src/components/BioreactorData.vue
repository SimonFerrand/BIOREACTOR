<template>
    <div>
        <h1>Bioreactor Data</h1>
        <p>{{ data }}</p>
    </div>
</template>

<script>
    import axios from 'axios'

    export default {
        data() {
            return {
                data: null
            }
        },
        mounted() {
            axios.get('http://192.168.1.25:8000/sensor_data')
                .then(response => {
                    this.data = response.data
                })
                .catch(error => {
                    console.log(error)
                })
        }
    }
</script>

<style scoped>
    /* Ajoutez ici votre style */
</style>
