public class ROT47 {
    /**
     * Metodo decryption: Permite transformar una cadena en hexadecimal que ha sido encriptada
     * con el algoritmo ROT47 que es un cipher tipo Cesar de 47 caracteres donde se reeemplaza
     * un caracter dentro del rango ASCII [33,126] rotando el caracter 47 posiciones diferentes
     * en la tabla ASCII. El algoritmo es invertible; es decir, aplicando el mismo algoritmo dos
     * veces se puede obtener el texto original.
     * 
     * El metodo devuelve una cadena en hexadecimal correspondiente a los bytes recibidos en el
     * backend de Sigfox, el cual debera ser parseado y convertido dependiendo de su uso.
     * 
     * Ejemplo: La cadena en hexadecimal 363733326133635f6264 se convierte en efba2b4035 al final
     * de la aplicacion del algoritmo.
     * 
     * @author                  Javier Arce
     * @author                  Rosa Pincay
     * 
     * @param hexadecimal       es la cadena en hexadecimal que se recibe desde la base de datos
     * @return                  devuelve la cadena en hexadecimal desencriptada
     * 
     * @since                   1.0
     */
    public String decryption(String hexadecimal) {
        String decrypText = "";
        int key = 47;
        StringBuilder output = new StringBuilder("");
        
        /**Cambio de HEX  String: Se parsea cada dos caracteres substring(i, i+2) de la cadena 
         * en hexadecimal recibida para anexarla 
         * a una cadena la cual contendra los caracteres correspondientes a dichos valores
         * en hexadecimal.
        */

        for (int i = 0; i < hexadecimal.length(); i+=2){
            String str = hexadecimal.substring(i, i + 2);
            output.append((char) Integer.parseInt(str, 16));
        }
        String message = output.toString();

        /**
         * Se lee la cadena obtenida de la conversion de hexadecimal hacia String tomando en cuenta
         * que los caracteres en ASCII empiezan desde 33 hasta el 126 por lo que se aumenta el valor 
         * de dicho caracter para cambiarlo.
         */

        for (int i = 0; i < message.length(); i++) {
            int temp = (int) message.charAt(i) - key;
            if ((int) message.charAt(i) == 32) {
                decrypText += " ";
            } else if (temp < 32) {
                temp += 94;
                decrypText += (char) temp;
            } else {
                decrypText += (char) temp;
            }
        }
        return decrypText;
    }

    //Prueba
    public static void main (String[] args){
        String hexadecimal = "363733326133635f6264";
        String salida = decryption(hexadecimal);
        System.out.println("Decrypted text: " + salida);
    }
}
